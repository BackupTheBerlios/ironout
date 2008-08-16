#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


static int parse_cmd(char *filename)
{
	free_node(parse(filename));
	return 0;
}

static int getname_cmd(char *filename, long offset)
{
	struct node *node = parse(filename);
	struct node *found = node_at(node, offset);
	if (found->type == AST_IDENTIFIER || found->type == AST_TYPENAME)
		puts(found->data);
	free_node(node);
	return 0;
}

static int checknode(struct node *node, void *data)
{
	if (node->type == AST_IDENTIFIER || node->type == AST_TYPENAME)
		if (!strcmp(node->data, (char *) data))
			printf("%ld %ld\n", node->start, node->end);
	return 1;
}

static int find_cmd(char *filename, long offset)
{
	struct node *node = parse(filename);
	struct node *found = node_at(node, offset);
	if (found->type == AST_IDENTIFIER || found->type == AST_TYPENAME)
		walk_nodes(node, checknode, found->data);
	free_node(node);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "parse") && argc > 2)
			return parse_cmd(argv[2]);
		if (!strcmp(argv[1], "getname") && argc > 3)
			return getname_cmd(argv[2], atoi(argv[3]));
		if (!strcmp(argv[1], "find") && argc > 3)
			return find_cmd(argv[2], atoi(argv[3]));
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
