#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "block.h"
#include "cfile.h"
#include "hash.h"
#include "name.h"


static int parse_cmd(char *filename)
{
	struct cfile *cfile = cfile_init(filename);
	if (cfile)
		cfile_free(cfile);
	return !!cfile;
}

static int getname_cmd(char *filename, long offset)
{
	struct cfile *cfile = cfile_init(filename);
	struct node *found = node_find(cfile->node, offset);
	if (found && (found->type == AST_IDENTIFIER ||
		      found->type == AST_TYPENAME))
		puts(found->data);
	cfile_free(cfile);
	return 0;
}

struct finddata {
	struct block *block;
	struct name *name;
};

static int checknode(struct node *node, void *data)
{
	struct finddata *finddata = data;
	if (node->type == AST_IDENTIFIER || node->type == AST_TYPENAME) {
		struct block *block = block_find(finddata->block, node->start);
		if (block_lookup(block, node) == finddata->name)
			printf("%ld %ld\n", node->start, node->end);
	}
	/* don't check nodes outside defining block */
	return 1;
}

static int find_cmd(char *filename, long offset)
{
	struct cfile *cfile = cfile_init(filename);
	struct node *node = node_find(cfile->node, offset);
	struct block *block = block_find(cfile->block, offset);
	struct name *name = block_lookup(block, node);
	if (name) {
		struct block *defblock = block_defining(block, node);
		if (defblock) {
			struct finddata finddata;
			finddata.block = defblock;
			finddata.name = name;
			node_walk(cfile->node, checknode, &finddata);
		}
	}
	cfile_free(cfile);
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
