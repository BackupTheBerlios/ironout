#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "block.h"


static int parse_cmd(char *filename)
{
	node_free(parse(filename));
	return 0;
}

static int getname_cmd(char *filename, long offset)
{
	struct node *node = parse(filename);
	struct node *found = node_find(node, offset);
	if (found && (found->type == AST_IDENTIFIER ||
		      found->type == AST_TYPENAME))
		puts(found->data);
	node_free(node);
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
	struct node *found = node_find(node, offset);
	if (found->type == AST_IDENTIFIER || found->type == AST_TYPENAME)
		node_walk(node, checknode, found->data);
	node_free(node);
	return 0;
}

static int block_cmd(char *filename, long offset)
{
	struct node *node = parse(filename);
	struct block *block = block_init(node);
	struct block *cur = block_find(block, offset);
	if (cur)
		printf("%ld %ld\n", cur->node->start, cur->node->end);
	block_free(block);
	node_free(node);
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
		if (!strcmp(argv[1], "block") && argc > 3)
			return block_cmd(argv[2], atoi(argv[3]));
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
