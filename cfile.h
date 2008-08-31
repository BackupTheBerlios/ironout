#ifndef _CFILE_H
#define _CFILE_H

#include "ast.h"
#include "block.h"

struct cfile {
	struct block *block;
	struct node *node;
	char *name;
};

struct cfile *cfile_init(char *filename);
void cfile_free(struct cfile *cfile);

#endif
