#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "hash.h"
#include "parse.h"
#include "utils.h"


static struct node *nodestack[1024];
static int nodecount = 0;
static struct hash *typedef_hash = NULL;

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
	if (!list_nodetype(node->type))
		return 0;
	while (cur->type == node->type) {
		result++;
		if (cur->count != 2)
			return result;
		if (cur->children[0]->type == node->type)
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
		struct node *old = NULL;
		struct node **newchildren = xmalloc(n * sizeof(struct node *));
		int start = 0, end = n;
		for (i = 0; i < n; i++) {
			if (cur->children[0]->type != node->type) {
				newchildren[start++] = cur->children[0];
				if (cur->count > 1)
					cur = cur->children[1];
			} else {
				newchildren[--end] = cur->children[1];
				cur = cur->children[0];
			}
			if (old)
				node_free_base(old, 0);
			old = cur;
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

struct node *parse(char *filename)
{
	FILE *file = fopen(filename, "r");
	if (!file)
		return NULL;
	yyrestart(file);
	yyparse();
	fclose(file);
	reset_tokenizer();
	if (nodecount != 1)
		printf("ERROR: %d nodes on the stack\n", nodecount);
	else
		linear_lists(nodestack[0]);
	hash_release(typedef_hash);
	typedef_hash = NULL;
	return nodestack[--nodecount];
}

static int str_cmp(void *data, void *key)
{
	return strcmp(data, key);
}

static void add_type(char *typename)
{
	if (!typedef_hash)
		typedef_hash = hash_init(str_hash, str_hash, str_cmp, 16);
	hash_put(typedef_hash, typename);
}

static int search_typedefs(struct node *node, void *data)
{
	int *istypedef = data;
	if (node->type == AST_TYPEDEFKW)
		*istypedef = 1;

	if (*istypedef && node->parent &&
	    node->type == AST_IDENTIFIER && node->parent->type == AST_DIRDECL)
		add_type(node->data);
	return 1;
}

struct node *push_node(enum nodetype type, long start, long end, int nchild)
{
	struct node *node = xmalloc(sizeof(struct node));
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
			if (node->type != AST_DECLSTMT &&
			    child->type == AST_DECLSTMT) {
				int istypedef = 0;
				node_walk(child, search_typedefs, &istypedef);
			}
		}
	}
	nodestack[nodecount++] = node;
	return node;
}

struct node *push_node_name(enum nodetype type, long start, long end, char *name)
{
	char *data = xmalloc(strlen(name) + 1);
	struct node *node = push_node(type, start, end, 0);
	strcpy(data, name);
	node->data = data;
	return node;
}

void node_free(struct node *node)
{
	node_free_base(node, 1);
}

int is_typename(char *name)
{
	add_type("FILE");
	return hash_get(typedef_hash, name) != NULL;
}
