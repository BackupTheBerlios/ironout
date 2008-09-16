#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "name.h"
#include "utils.h"
#include "strutils.h"

struct name *name_init(char *name, int flags)
{
	struct name *result = xmalloc(sizeof(struct name));
	result->name = name;
	result->flags = flags;
	return result;
}

void name_free(struct name *name)
{
	free(name);
}

static int struct_node(struct node *node)
{
	if (!node)
		return 0;
	switch (node->type) {
	case AST_DIRDECL:
	case AST_DECL:
	case AST_DECLLIST:
	case AST_DECLSPEC:
	case AST_DECLSTMT:
		return struct_node(node->parent);
	case AST_STRUCTBITS:
	case AST_STRUCTDECLLIST:
		return 1;
	default:
		return 0;
	}
}

int node_isfield(struct node *node)
{
	struct node *cur = node->parent;
	if (!cur)
		return 0;
	if ((cur->type == AST_GETATTR || cur->type == AST_DEREF)
	    && cur->children[1] == node)
		return 1;
	if (struct_node(cur))
		return 1;
	return 0;
}

static int node_insidedecl(struct node *node)
{
	while (node) {
		switch (node->type) {
		case AST_DECL:
		case AST_DIRDECL:
		case AST_PARAMDECL:
		case AST_PARAMLIST:
		case AST_IDLIST:
			break;
		default:
			return node->type != AST_FUNCTION;
		}
		node = node->parent;
	}
	return 1;
}

static int node_isparamdecl(struct node *node)
{
	struct node *cur = node;
	while (cur) {
		switch (cur->type) {
		case AST_DIRDECL:
		case AST_DECL:
		case AST_IDENTIFIER:
			cur = cur->parent;
			break;
		case AST_IDLIST:
		case AST_PARAMDECL:
			return node_insidedecl(cur);
		default:
			return 0;
		}
	}
	return 0;
}

int guess_name_flags(struct node *node)
{
	int flags = 0;
	struct node *parent = node->parent;
	if (parent) {
		if (parent->type == AST_ENUM)
			flags |= NAME_ENUM;
		if (parent->type == AST_STRUCT) {
			enum nodetype type = parent->children[0]->type;
			if (type == AST_STRUCTKW)
				flags |= NAME_STRUCT;
			else
				flags |= NAME_UNION;
		}
		if (parent->type == AST_LABELED ||
		    parent->type == AST_GOTO)
			flags |= NAME_LABEL;
	}
	if (node_isfield(node))
		flags |= NAME_FIELD;
	if (node_isparamdecl(node))
		flags |= NAME_PARAMDECL;
	return flags;
}

int modifiers_match(struct name *name, int flags)
{
	return !((name->flags ^ flags) & NAME_MOD_MASK);
}

static struct name *name_at(struct project *project,
			    struct cfile *cfile, long offset)
{
	struct node *node;
	struct block *block;
	struct name *name;
	int i;
	node = node_find(cfile->node, offset);
	if (!node)
		return NULL;
	block = block_find(cfile->block, offset);
	if (!block)
		return NULL;
	name = block_lookup(block, node);
	if (name)
		return name;
	for (i = 0; i < project->count; i++) {
		struct cfile *cfile = project->files[i];
		name = block_lookup(cfile->block, node);
		if (name && !(name->flags & NAME_STATIC))
			return name;
	}
	return NULL;
}

struct {
	char *name;
	int flag;
} tags[] = {{"struct ", NAME_STRUCT},
	    {"enum ", NAME_ENUM},
	    {"union ", NAME_UNION},
	    {"label ", NAME_LABEL}};

static struct name *token_name(char *token)
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
	return name_init(token, flags);
}

static struct block *block_byname(struct block *block, char *name)
{
	struct block_list *blist = block_children(block);
	while (blist) {
		struct node *node = blist->block->node;
		if (node->type == AST_FUNCTION &&
		    !strcmp(name, declarator_name(node->children[1])))
			return blist->block;
		blist = blist->next;
	}
	return NULL;
}

static struct name *name_hier(struct block *block, char *location)
{
	char token[128];
	char *newlocation = readtoken(token, location, ":");
	struct block_list *blist = NULL;
	if (*newlocation) {
		struct block *newblock = block_byname(block, token);
		if (newblock)
			return name_hier(newblock, newlocation);
	} else {
		struct name *key = token_name(token);
		struct name *name = hash_get(block_names(block), key);
		name_free(key);
		if (name)
			return name;
	}
	blist = block_children(block);
	while (blist) {
		struct name *name = name_hier(blist->block, location);
		if (name)
			return name;
		blist = blist->next;
	}
	return NULL;
}

struct name *name_find(struct project *project,
		       struct cfile *cfile, char *location)
{
	if (isdigit(location[0]))
		return name_at(project, cfile, atoi(location));
	return name_hier(cfile->block, location);
}
