#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "block.h"
#include "hash.h"


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

struct finddata {
	char *name;
	long start, end;
};

static int checknode(struct node *node, void *data)
{
	struct finddata *finddata = data;
	if (node->type == AST_IDENTIFIER || node->type == AST_TYPENAME)
		if (!strcmp(node->data, finddata->name))
			printf("%ld %ld\n", node->start, node->end);
	return finddata->start <= node->end && node->start < finddata->end;
}

static int find_cmd(char *filename, long offset)
{
	struct node *node = parse(filename);
	struct block *block = block_init(node);
	struct node *cnode = node_find(node, offset);
	if (cnode->type == AST_IDENTIFIER || cnode->type == AST_TYPENAME) {
		struct finddata finddata;
		struct block *cblock = block_find(block, offset);
		struct block *defblock = block_defining(cblock, cnode->data);
		finddata.start = defblock->node->start;
		finddata.end = defblock->node->end;
		finddata.name = cnode->data;
		node_walk(node, checknode, &finddata);
	}
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
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
