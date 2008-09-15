#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "hash.h"
#include "parse.h"
#include "utils.h"


static struct node *nodestack[1024];
static int nodecount = 0;
static int parse_error = 0;

static int list_nodetype(enum nodetype nodetype)
{
	switch (nodetype) {
	case AST_ARGLIST:
	case AST_BLOCKLIST:
	case AST_BODY:
	case AST_DECLLIST:
	case AST_DECLSPEC:
	case AST_DESIGLIST:
	case AST_ENUMLIST:
	case AST_EXPRLIST:
	case AST_FILE:
	case AST_INITIALIZERLIST:
	case AST_INITLIST:
	case AST_PARAMLIST:
	case AST_STRUCTDECLLIST:
	case AST_STRUCTLIST:
	case AST_STRUCTQUALLIST:
	case AST_TYPEQUALLIST:
		return 1;
	default:
		return 0;
	}
}

static int list_size(struct node *node)
{
	int result = 0;
	struct node *cur = node;
	int preorder;
	if (!list_nodetype(node->type) || !node->count)
		return 0;
	preorder = cur->children[0]->type == node->type;
	while (cur->type == node->type) {
		result++;
		if (cur->count != 2)
			return result;
		if (preorder)
			cur = cur->children[0];
		else
			cur = cur->children[1];
	}
	return result;
}

static void node_free_base(struct node *node, int children)
{
	if (children && node->children) {
		int i;
		for (i = 0; i < node->count; i++)
			node_free(node->children[i]);
		free(node->children);
	}
	if (node->data)
		free(node->data);
	free(node);
}

static void linear_lists(struct node *node)
{
	int n = list_size(node);
	int i;
	if (n) {
		struct node *cur = node;
		struct node **newchildren = xmalloc(n * sizeof(struct node *));
		int preorder = cur->children[0]->type == node->type;
		for (i = 0; i < n; i++) {
			struct node *old = cur;
			if (preorder) {
				if (cur->count > 1)
					newchildren[n - i - 1] = cur->children[1];
				else
					newchildren[n - i - 1] = cur->children[0];
				cur = cur->children[0];
			} else {
				newchildren[i] = cur->children[0];
				cur = cur->children[cur->count - 1];
			}
			if (old != node && i < n - 1)
				node_free_base(old, 0);
		}
		for (i = 0; i < n; i++)
			newchildren[i]->parent = node;
		free(node->children);
		node->children = newchildren;
		node->count = n;
	}
	for (i = 0; i < node->count; i++)
		linear_lists(node->children[i]);
}

static void error_happened()
{
	int i;
	parse_error = 1;
	for (i = 0; i < nodecount; i++)
		node_free(nodestack[i]);
	nodecount = 0;
}

struct node *parse(char *filename)
{
	FILE *file = fopen(filename, "r");
	if (!file)
		return NULL;
	parse_error = 0;
	yyrestart(file);
	yyparse();
	fclose(file);
	reset_tokenizer();
	if (parse_error || nodecount != 1) {
		fprintf(stderr, "ERROR: %d nodes on the stack\n", nodecount);
		error_happened();
		return NULL;
	}
	linear_lists(nodestack[0]);
	return nodestack[--nodecount];
}

static int str_cmp(void *data, void *key)
{
	return strcmp(data, key);
}

struct node *push_node(enum nodetype type, long start, long end, int nchild)
{
	struct node *node;
	if (parse_error || nchild > nodecount) {
		error_happened();
		return NULL;
	}
	node = xmalloc(sizeof(struct node));
	memset(node, 0, sizeof(struct node));
	node->type = type;
	node->start = start;
	node->end = end;
	if (nchild) {
		int i;
		node->count = nchild;
		node->children = xmalloc(nchild * sizeof(struct node *));
		for (i = nchild - 1; i >= 0; i--) {
			struct node *child = nodestack[--nodecount];
			node->children[i] = child;
			child->parent = node;
		}
	}
	nodestack[nodecount++] = node;
	return node;
}

struct node *push_node_name(enum nodetype type, long start, long end, char *name)
{
	struct node *node = push_node(type, start, end, 0);
	if (parse_error || !node)
		return NULL;
	node->data = xmalloc(strlen(name) + 1);
	strcpy(node->data, name);
	return node;
}

struct node *push_decl(enum nodetype type, long start, long end,
			    int nchild, enum decltype decltype)
{
	struct node *node = push_node(type, start, end, nchild);
	if (parse_error || !node)
		return NULL;
	node->data = xmalloc(sizeof(decltype));
	*(enum decltype *) node->data = decltype;
	return node;
}

void node_free(struct node *node)
{
	node_free_base(node, 1);
}

static struct hash *typedefs = NULL;

static void add_type(char *typename)
{
	if (!typedefs)
		typedefs = hash_init(str_hash, str_hash, str_cmp, 16);
	hash_put(typedefs, typename);
}

int is_typename(char *name)
{
	if (!typedefs) {
		add_type("FILE");
		add_type("size_t");
		add_type("DIR");
	}
	return hash_get(typedefs, name) != NULL;
}
