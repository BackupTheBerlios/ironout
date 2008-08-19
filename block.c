#include <stdlib.h>
#include <string.h>
#include "block.h"


struct block *block_init(struct node *node)
{
	struct block *block = malloc(sizeof(struct block));
	block->node = node;
	block->children = NULL;
	return block;
}

void block_free(struct block *block)
{
	struct block_list *cur = block->children;
	while (cur) {
		struct block_list *next = cur->next;
		block_free(cur->block);
		free(cur);
		cur = next;
	}
	free(block);
}

int find_blocks(struct node *node, void *data)
{
	struct block *block = data;
	if (node->type == AST_BLOCK && node != block->node) {
		struct block_list *newchild = malloc(sizeof(struct block_list));
		newchild->block = block_init(node);
		newchild->next = block->children;
		block->children = newchild;
		return 0;
	}
	return 1;
}

static void init_children(struct block *block)
{
	if (!block->children)
		walk_nodes(block->node, find_blocks, block);
}

static void block_walk(struct block *block,
		int (*see)(struct block *, void *),
		void *data)
{
	if (see(block, data)) {
		struct block_list *cur;
		if (!block->children)
			init_children(block);
		cur = block->children;
		while (cur) {
			block_walk(cur->block, see, data);
			cur = cur->next;
		}
	}
}

struct block_search {
	long offset;
	struct block *result;
};

int search_offset(struct block *block, void *data)
{
	struct block_search *search = data;
	long offset = search->offset;
	if (block->node->start <= offset && offset < block->node->end) {
		search->result = block;
		return 1;
	}
	return 0;
}

struct block *block_find(struct block *block, long offset)
{
	struct block_search result;
	result.offset = offset;
	result.result = NULL;
	block_walk(block, search_offset, &result);
	return result.result;
}
