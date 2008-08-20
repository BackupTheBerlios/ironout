#include <stdlib.h>
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
