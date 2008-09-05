#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "cfile.h"
#include "utils.h"


struct cfile *cfile_init(char *filename)
{
	struct cfile *cfile;
	struct node *node = parse(filename);
	if (!node)
		return NULL;
	cfile = xmalloc(sizeof(struct cfile));
	cfile->name = xmalloc(strlen(filename) + 1);
	strcpy(cfile->name, filename);
	cfile->node = node;
	cfile->block = block_init(cfile->node);
	return cfile;
}

void cfile_free(struct cfile *cfile)
{
	block_free(cfile->block);
	node_free(cfile->node);
	free(cfile->name);
}
