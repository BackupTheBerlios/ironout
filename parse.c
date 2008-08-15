#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parse.h"


static struct node *nodestack[1024];
static int nodecount = 0;

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
		printf("WARNING: %d nodes on the stack\n", nodecount);
	return nodestack[--nodecount];
}

void push_node(enum nodetype type, long start, long end, int nchild)
{
	int i = 0;
	struct node *node = malloc(sizeof(struct node));
	memset(node, 0, sizeof(struct node));
	node->type = type;
	node->start = start;
	node->end = end;
	node->count = nchild;
	node->children = malloc(nchild * sizeof(struct node *));
	for (i = 0; i < nchild; i++)
		node->children[i] = nodestack[--nodecount];
	nodestack[nodecount++] = node;
}
