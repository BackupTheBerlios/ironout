#ifndef _BLOCK_H
#define _BLOCK_H

#include "ast.h"

struct block_list {
	struct block_list *next;
	struct block *block;
};

struct block {
	struct node *node;
	struct block_list *children;
};

struct block *block_init(struct node *node);
void block_free(struct block *block);
struct block *block_find(struct block *block, long offset);

#endif
