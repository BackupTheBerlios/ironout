#include <stdlib.h>
#include "name.h"
#include "utils.h"

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

int modifier_flags(struct node *node)
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
	return flags;
}

int modifiers_match(struct name *name, int flags)
{
	return !((name->flags ^ flags) & NAME_MOD_MASK);
}

struct name *name_find(struct project *project,
		       struct cfile *cfile, char *location)
{
	struct node *node;
	struct block *block;
	struct name *name;
	int i;
	long offset = atoi(location);
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
