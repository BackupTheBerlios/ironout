#ifndef _BLOCK_H
#define _BLOCK_H

#include "ast.h"
#include "name.h"

struct block_list {
	struct block_list *next;
	struct block *block;
};

struct block {
	struct node *node;
	struct block *parent;
	struct block_list *_children;
	struct hash *_names;
	unsigned walked:1;
};

struct block *block_init(struct node *node);
void block_free(struct block *block);
struct block *block_find(struct block *block, long offset);
struct hash *block_names(struct block *block);
struct block_list *block_children(struct block *block);
struct block *block_defining(struct block *block, struct node *node);
struct name *block_lookup(struct block *block, struct node *node);
struct name *block_find_hier(struct block *block, char *location);

#endif
