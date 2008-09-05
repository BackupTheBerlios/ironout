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
	}
	return flags;
}

int modifiers_match(struct name *name, int flags)
{
	return !((name->flags ^ flags) & NAME_MOD_MASK);
}
