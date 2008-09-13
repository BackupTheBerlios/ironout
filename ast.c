#include <stdlib.h>
#include <string.h>
#include "ast.h"


void node_walk(struct node *node,
	       int (*callback) (struct node *, void *),
	       void *data)
{
	int i;
	if (callback(node, data))
		for (i = 0; i < node->count; i++)
			node_walk(node->children[i], callback, data);
}

struct node_search {
	long offset;
	struct node *result;
};

static int search_node(struct node *node, void *data)
{
	struct node_search *search = data;
	long offset = search->offset;
	if (node->start <= offset && offset < node->end) {
		search->result = node;
		return 1;
	}
	return 0;
}

struct node *node_find(struct node *node, long offset)
{
	struct node_search search;
	search.offset = offset;
	search.result = NULL;
	node_walk(node, search_node, &search);
	return search.result;
}

int node_cmp(struct node *n1, struct node *n2)
{
	int i;
	if (n1->type != n2->type || n1->count != n2->count)
		return 1;
	/* node->data should be compared based on node->type */
	if (n1->type == AST_IDENTIFIER || n1->type == AST_TYPENAME)
		if (strcmp(n1->data, n2->data))
			return 1;
	for (i = 0; i < n1->count; i++)
		if (node_cmp(n1->children[i], n2->children[i]))
			return 1;
	return 0;
}

static int struct_node(struct node *node)
{
	if (!node)
		return 0;
	switch (node->type) {
	case AST_DIRDECL:
	case AST_DECL:
	case AST_DECLLIST:
	case AST_DECLSPEC:
	case AST_DECLSTMT:
		return struct_node(node->parent);
	case AST_STRUCTBITS:
	case AST_STRUCTDECLLIST:
		return 1;
	default:
		return 0;
	}
}

int node_isfield(struct node *node)
{
	struct node *cur = node->parent;
	if (!cur)
		return 0;
	if ((cur->type == AST_GETATTR || cur->type == AST_DEREF)
	    && cur->children[1] == node)
		return 1;
	if (struct_node(cur))
		return 1;
	return 0;
}

char *declarator_name(struct node *node)
{
	struct node *cur = node;
	while (cur->count)
		if (cur->type == AST_PTR)
			cur = cur->children[cur->count - 1];
		else
			cur = cur->children[0];
	if (cur->type == AST_IDENTIFIER)
		return cur->data;
	return NULL;
}
