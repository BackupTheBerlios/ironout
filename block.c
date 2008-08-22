#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "hash.h"
#include "utils.h"


struct block *block_init(struct node *node)
{
	struct block *block = xmalloc(sizeof(struct block));
	block->node = node;
	block->children = NULL;
	block->names = NULL;
	block->parent = NULL;
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
		struct block_list *newchild = xmalloc(sizeof(struct block_list));
		newchild->block = block_init(node);
		newchild->block->parent = block;
		newchild->next = block->children;
		block->children = newchild;
		return 0;
	}
	return 1;
}

static void init_children(struct block *block)
{
	node_walk(block->node, find_blocks, block);
	block->walked = 1;
}

static void block_walk(struct block *block,
		int (*see)(struct block *, void *),
		void *data)
{
	if (see(block, data)) {
		struct block_list *cur;
		if (!block->walked)
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

static int search_offset(struct block *block, void *data)
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

static int _strcmp(void *data, void *key)
{
	return strcmp(data, key);
}

static int decl_node(struct node *node)
{
	switch (node->type) {
	case AST_DECLLIST:
	case AST_DECLSTMT:
	case AST_INITLIST:
	case AST_INIT:
	case AST_DECL:
	case AST_DECL2:
	case AST_DECLSPEC:
	case AST_TYPE:
	case AST_ENUMVAL:
		return 1;
	case AST_ENUM:
		return node->children[node->count - 1]->type == AST_ENUMLIST;
	default:
		return 0;
	}
}

static int find_names(struct node *node, void *data)
{
	struct block *block = data;
	if (node->type == AST_IDENTIFIER)
		hash_put(block->names, node->data);
	return block->node == node || decl_node(node);
}

static void init_names(struct block *block)
{
	block->names = hash_init(str_hash, str_hash, _strcmp, 4);
	node_walk(block->node, find_names, block);
}

struct hash *block_names(struct block *block)
{
	if (!block->names)
		init_names(block);
	return block->names;
}

struct block *block_defining(struct block *block, char *name)
{
	while (block && !hash_get(block_names(block), name))
		block = block->parent;
	return block;
}
