#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "hash.h"
#include "name.h"
#include "strutils.h"
#include "utils.h"

struct ref {
	char *name;
	int flags;
};

struct ref *ref_init(char *name, int flags)
{
	struct ref *result = xmalloc(sizeof(struct ref));
	result->name = name;
	result->flags = flags;
	return result;
}

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
	struct block_list *cur = block->_children;
	while (cur) {
		struct block_list *next = cur->next;
		block_free(cur->block);
		free(cur);
		cur = next;
	}
	if (block->_names) {
		hash_walk(block->_names, free_hashed, NULL);
		hash_release(block->_names);
	}
	free(block);
}

static int find_blocks(struct node *node, void *data)
{
	struct block *block = data;
	if (node == block->node)
		return 1;
	if (node->type == AST_BLOCK || node->type == AST_FUNCTION) {
		struct block_list *newchild = xmalloc(sizeof(struct block_list));
		newchild->block = block_init(node);
		newchild->block->parent = block;
		newchild->next = block->_children;
		block->_children = newchild;
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
	cur = block->_children;
	while (cur) {
		struct block_list *next = cur->next;
		cur->next = newhead;
		newhead = cur;
		cur = next;
	}
	block->_children = newhead;
	block->walked = 1;
}

struct block_list *block_children(struct block *block)
{
	if (!block->walked)
		init_children(block);
	return block->_children;
}

static void block_walk(struct block *block,
		int (*see)(struct block *, void *),
		void *data)
{
	if (see(block, data)) {
		struct block_list *cur;
		cur = block_children(block);
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

static void handle_enum(struct block *block, struct node *node)
{
	int i;
	/* ENUM -> ENUMLIST */
	struct node *list = node->children[node->count - 1];
	if (list->type != AST_ENUMLIST)
		return;
	if (node->children[0]->type == AST_IDENTIFIER)
		hash_put(block_names(block),
			 name_init(node->children[0], NAME_ENUM));
	for (i = 0; i < list->count; i++) {
		/* ENUMLIST -> ENUMVAL -> IDENTIFIER */
		struct node *child = list->children[i]->children[0];
		hash_put(block_names(block),
			 name_init(child, 0));
	}
}

static void handle_struct(struct block *block, struct node *node)
{
	if (node->count < 3)
		return;
	if (node->children[1]->type == AST_IDENTIFIER) {
		int flags = node->children[0]->type == AST_STRUCTKW ?
			NAME_STRUCT : NAME_UNION;
		hash_put(block_names(block),
			 name_init(node->children[1], flags));
	}
}

struct declinfo {
	struct block *block;
	int flags;
	unsigned isstatic : 1;
	unsigned isextern : 1;
};

static int analyze_declspec(struct node *node, void *data)
{
	struct declinfo *declinfo = data;
	switch (node->type) {
	case AST_ENUM:
		handle_enum(declinfo->block, node);
		return 0;
	case AST_STRUCT:
		handle_struct(declinfo->block, node);
		return 0;
	case AST_STATICKW:
		declinfo->isstatic = 1;
		return 0;
	case AST_EXTERNKW:
		declinfo->isextern = 1;
		return 0;
	default:
		return 1;
	}
}

static int declinfo_flags(struct declinfo *declinfo)
{
	int flags = 0;
	if (declinfo->isstatic)
		flags |= NAME_STATIC;
	if (declinfo->isextern)
		flags |= NAME_EXTERN;
	return flags | declinfo->flags;
}

static void handle_function(struct block *block, struct node *node)
{
	struct node *name;
	struct declinfo declinfo;
	memset(&declinfo, 0, sizeof(declinfo));
	node_walk(node->children[0], analyze_declspec, &declinfo);
	name = declarator_name(node->children[1]);
	if (name) {
		int flags = declinfo_flags(&declinfo) | NAME_FUNCTION;
		hash_put(block_names(block), name_init(name, flags));
	}
}

static void add_declarator_name(struct declinfo *declinfo, struct node *decl)
{
	struct node *name;
	struct block *block = declinfo->block;
	struct node *dirdecl = decl->children[decl->count - 1];
	/* nothing to do for function declarations */
	switch (*(enum decltype *) dirdecl->data) {
	case DECL_PARENS:
	case DECL_PARENS_TYPE:
	case DECL_PARENS_ID:
		return;
	default:
		break;
	}
	name = declarator_name(decl);
	if (name)
		hash_put(block_names(block),
			 name_init(name, declinfo_flags(declinfo)));
}

static int search_declarators(struct node *node, void *data)
{
	struct declinfo *declinfo = data;
	if (node->type == AST_INIT) {
		add_declarator_name(declinfo, node->children[0]);
		return 0;
	}
	/* should ignore "extern int var" but not "extern int var = 1" */
	if (!declinfo->isextern && node->type == AST_DECL) {
		add_declarator_name(declinfo, node);
		return 0;
	}
	return 1;
}

static void handle_declaration(struct block *block,
			       struct node *node,
			       int flags)
{
	struct declinfo declinfo;
	memset(&declinfo, 0, sizeof(declinfo));
	declinfo.block = block;
	declinfo.flags = flags;
	node_walk(node->children[0], analyze_declspec, &declinfo);
	if (node->count > 1)
		node_walk(node->children[1], search_declarators, &declinfo);
}

static void handle_parameters(struct block *block, struct node *node)
{
	int i;
	for (i = 0; i < node->count; i++) {
		struct node *param = node->children[i];
		if (param->count > 1) {
			struct node *name = declarator_name(
				param->children[param->count - 1]);
			if (name)
				hash_put(block_names(block),
					 name_init(name, 0));
		}
	}
}

static void handle_label(struct block *block, struct node *node)
{
	struct node *id = node->children[0];
	hash_put(block_names(block), name_init(id, NAME_LABEL));
}

static int find_names(struct node *node, void *data)
{
	struct block *block = data;
	switch (node->type) {
	case AST_FUNCTION:
		handle_function(block, node);
		break;
	case AST_DECLSTMT:
		handle_declaration(block, node, 0);
		break;
	case AST_LABELED:
		handle_label(block, node);
		return 1;
	case AST_BLOCKLIST:
	case AST_DECLLIST:
		return 1;
	default:
		break;
	}
	return block->node == node;
}

static long name_hash(void *name)
{
	return str_hash(((struct name *) name)->name);
}

static long ref_hash(void *name)
{
	return str_hash(((struct name *) name)->name);
}

static int name_cmp(void *data, void *key)
{
	struct name *name = data;
	struct ref *ref = key;
	if (!strcmp(name->name, ref->name))
		return !modifiers_match(name, ref->flags);
	return 1;
}

static void init_names(struct block *block)
{
	block->_names = hash_init(name_hash, ref_hash, name_cmp, 4);
	if (block->node->type == AST_FUNCTION) {
		/* FUNCTION -> DECL -> DECL2 -> PARAMLIST */
		struct node *node = block->node;
		struct node *decl = node->children[node->count - 2];
		struct node *decl2 = decl->children[decl->count - 1];
		if (decl2->count > 1) {
			struct node *params = decl2->children[decl2->count - 1];
			handle_parameters(block, params);
		}
		if (node->count >= 4) {
			int i;
			struct node *decllist = node->children[node->count - 2];
			for (i = 0; i < decllist->count; i++)
				handle_declaration(block,
						   decllist->children[i],
						   NAME_PARAM);
		}
	} else {
		node_walk(block->node, find_names, block);
	}
}

struct hash *block_names(struct block *block)
{
	if (!block->_names)
		init_names(block);
	return block->_names;
}

struct ref *name_on(struct node *node)
{
	if (node->type != AST_IDENTIFIER && node->type != AST_TYPENAME)
		return NULL;
	return ref_init(node->data, guess_name_flags(node));
}

struct block *block_defining(struct block *block, struct node *node)
{
	struct ref *ref = name_on(node);
	while (block && !hash_get(block_names(block), ref))
		block = block->parent;
	free(ref);
	return block;
}

struct name *block_lookup(struct block *block, struct node *node)
{
	struct ref *ref = name_on(node);
	struct name *result = NULL;
	if (!ref || ref->flags & (NAME_FIELD | NAME_PARAMDECL))
		return NULL;
	while (block) {
		struct name *cur = hash_get(block_names(block), ref);
		if (cur) {
			result = cur;
			break;
		}
		block = block->parent;
	}
	free(ref);
	return result;
}

struct {
	char *name;
	int flag;
} tags[] = {{"struct ", NAME_STRUCT},
	    {"enum ", NAME_ENUM},
	    {"union ", NAME_UNION},
	    {"label ", NAME_LABEL}};

static struct ref *token_name(char *token)
{
	int flags = 0;
	int i;
	for (i = 0; i < LENGTH(tags); i++) {
		if (startswith(token, tags[i].name)) {
			token += strlen(tags[i].name);
			flags |= tags[i].flag;
			break;
		}
	}
	return ref_init(token, flags);
}

static struct block *block_byname(struct block *block, char *name)
{
	struct block_list *blist = block_children(block);
	while (blist) {
		struct node *node = blist->block->node;
		struct node *declname = declarator_name(node->children[1]);
		if (node->type == AST_FUNCTION &&
		    !strcmp(name, declname->data))
			return blist->block;
		blist = blist->next;
	}
	return NULL;
}

struct name *block_find_hier(struct block *block, char *location)
{
	char token[128];
	char *newlocation = readtoken(token, location, ":");
	struct block_list *blist = NULL;
	if (*newlocation) {
		struct block *newblock = block_byname(block, token);
		if (newblock)
			return block_find_hier(newblock, newlocation);
	} else {
		struct ref *key = token_name(token);
		struct name *name = hash_get(block_names(block), key);
		free(key);
		if (name)
			return name;
	}
	blist = block_children(block);
	while (blist) {
		struct name *name = block_find_hier(blist->block, location);
		if (name)
			return name;
		blist = blist->next;
	}
	return NULL;
}
