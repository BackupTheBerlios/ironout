#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parse.h"
#include "utils.h"


static struct node *nodestack[1024];
static int nodecount = 0;

static enum nodetype listnodes[] = {
	AST_FILE, AST_BODY, AST_IDLIST, AST_ENUMLIST,
	AST_INITIALIZERLIST, AST_STRUCTLIST, AST_STMTLIST,
	AST_SPECLIST, AST_EXPRLIST, AST_DECLLIST, AST_ARGLIST};

static int list_size(struct node *node)
{
	int result = 0;
	struct node *cur = node;
	int found = 0;
	int i;
	for (i = 0; i < LENGTH(listnodes); i++)
		if (node->type == listnodes[i]) {
			found = 1;
			break;
		}
	if (!found)
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

static void linear_lists(struct node *node)
{
	int n = list_size(node);
	int i;
	if (n) {
		struct node *cur = node;
		struct node **newchildren = malloc(n * sizeof(struct node *));
		for (i = 0; i < n; i++) {
			if (cur->children[0]->type != node->type) {
				newchildren[i] = cur->children[0];
				if (cur->count > 1)
					cur = cur->children[1];
			} else {
				newchildren[i] = cur->children[1];
				cur = cur->children[0];
			}
		}
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
		printf("WARNING: %d nodes on the stack\n", nodecount);
	linear_lists(nodestack[0]);
	return nodestack[--nodecount];
}

void push_node(enum nodetype type, long start, long end, int nchild)
{
	struct node *node = malloc(sizeof(struct node));
	memset(node, 0, sizeof(struct node));
	node->type = type;
	node->start = start;
	node->end = end;
	if (nchild) {
		int i;
		node->count = nchild;
		node->children = malloc(nchild * sizeof(struct node *));
		for (i = 0; i < nchild; i++)
			node->children[i] = nodestack[--nodecount];
	}
	nodestack[nodecount++] = node;
}

void free_node(struct node *node)
{
	if (node->children) {
		int i;
		for (i = 0; i < node->count; i++)
			free_node(node->children[i]);
		free(node->children);
	}
	if (node->data)
		free(node->data);
	free(node);
}

void node_at(struct node *node, long offset)
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
	if (cur->type == AST_IDENTIFIER)
		printf("identifier found!\n");
}
