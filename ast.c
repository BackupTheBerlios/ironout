#include "ast.h"


struct node *node_at(struct node *node, long offset)
{
	struct node *cur = node;
	int i;
	int found = 1;
	while (found) {
		found = 0;
		for (i = 0; i < cur->count; i++) {
			if (cur->children[i]->start <= offset &&
			    offset < cur->children[i]->end) {
				cur = cur->children[i];
				found = 1;
				break;
			}
		}
	}
	return cur;
}

void walk_nodes(struct node *node,
		int (*callback) (struct node *, void *), void *data)
{
	int i;
	if (callback(node, data))
		for (i = 0; i < node->count; i++)
			walk_nodes(node->children[i], callback, data);
}