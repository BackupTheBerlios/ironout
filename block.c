#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "hash.h"
#include "name.h"
#include "utils.h"


struct block *block_init(struct node *node)
{
	struct block *block = xmalloc(sizeof(struct block));
	memset(block, 0, sizeof(*block));
	block->node = node;
	return block;
}

static void free_hashed(void *name, void *data)
{
	free(name);
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
	if (block->names) {
		hash_walk(block->names, free_hashed, NULL);
		hash_release(block->names);
	}
	free(block);
}

static int find_blocks(struct node *node, void *data)
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
	struct block_list *cur = NULL;
	struct block_list *newhead = NULL;
	node_walk(block->node, find_blocks, block);
	/* reversing children order */
	cur = block->children;
	while (cur) {
		struct block_list *next = cur->next;
		cur->next = newhead;
		newhead = cur;
		cur = next;
	}
	block->children = newhead;
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

static int decl_node(struct node *node)
{
	switch (node->type) {
	case AST_DECL2:
	case AST_DECL:
	case AST_DECLLIST:
	case AST_DECLSPEC:
	case AST_DECLSTMT:
	case AST_FILE:
	case AST_INIT:
	case AST_INITLIST:
	case AST_TYPE:
		return 1;
	default:
		return 0;
	}
}

static void handle_enum(struct block *block, struct node *node)
{
	int i;
	/* ENUM -> ENUMLIST */
	struct node *list = node->children[node->count - 1];
	if (list->type != AST_ENUMLIST)
		return;
	if (node->children[0]->type == AST_IDENTIFIER)
		hash_put(block_names(block),
			 name_init(node->children[0]->data, NAME_ENUM));
	for (i = 0; i < list->count; i++) {
		/* ENUMLIST -> ENUMVAL -> IDENTIFIER */
		struct node *child = list->children[i]->children[0];
		hash_put(block_names(block),
			 name_init(child->data, 0));
	}
}

static int find_names(struct node *node, void *data)
{
	struct block *block = data;
	if (node->type == AST_IDENTIFIER)
		hash_put(block_names(block), name_init(node->data, 0));
	if (node->type == AST_ENUM)
		handle_enum(block, node);
	if (node->type == AST_STRUCT && node->count >= 3)
		if (node->children[1]->type == AST_IDENTIFIER) {
			int flags = node->children[0]->type == AST_STRUCTKW ?
				NAME_STRUCT : NAME_UNION;
			hash_put(block_names(block),
				 name_init(node->children[1]->data, flags));
		}
	return block->node == node || decl_node(node);
}

long name_hash(void *name)
{
	return str_hash(((struct name *) name)->name);
}

int name_cmp(void *data, void *key)
{
	struct name *n1 = data;
	struct name *n2 = key;
	if (!strcmp(n1->name, n2->name))
		return (n1->flags ^ n2->flags) & NAME_MOD_MASK;
	return 1;
}

static void init_names(struct block *block)
{
	block->names = hash_init(name_hash, name_hash, name_cmp, 4);
	node_walk(block->node, find_names, block);
}

struct hash *block_names(struct block *block)
{
	if (!block->names)
		init_names(block);
	return block->names;
}

struct name *name_on(struct node *node)
{
	int flags = 0;
	if (node->type != AST_IDENTIFIER && node->type != AST_TYPENAME)
		return NULL;
	if (node->parent) {
		if (node->parent->type == AST_ENUM)
			flags |= NAME_ENUM;
		if (node->parent->type == AST_STRUCT) {
			enum nodetype type = node->parent->children[0]->type;
			if (type == AST_STRUCTKW)
				flags |= NAME_STRUCT;
			else
				flags |= NAME_UNION;
		}
	}
	return name_init(node->data, flags);
}

struct block *block_defining(struct block *block, struct node *node)
{
	struct name *name = name_on(node);
	while (block && !hash_get(block_names(block), name))
		block = block->parent;
	name_free(name);
	return block;
}

struct name *block_lookup(struct block *block, struct node *node)
{
	struct name *name = name_on(node);
	struct name *result = NULL;
	while (block) {
		struct name *cur = hash_get(block_names(block), name);
		if (cur) {
			result = cur;
			break;
		}
		block = block->parent;
	}
	name_free(name);
	return result;
}
