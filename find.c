#include <stdlib.h>
#include <string.h>
#include "find.h"
#include "utils.h"


struct finddata {
	struct cfile *cfile;
	struct block *block;
	struct name *name;
	struct occurrence *occurrence;
};

static int extern_node_matches(struct name *name, struct node *node)
{
	if (!modifiers_match(name, modifier_flags(node)) ||
	    name->flags & NAME_STATIC)
		return 0;
	return !strcmp(name->name, node->data) && !node_isfield(node);
}

static int does_match(struct node *node, void *data)
{
	struct finddata *finddata = data;
	if (node->type == AST_IDENTIFIER || node->type == AST_TYPENAME) {
		struct block *block = block_find(finddata->block, node->start);
		struct name *name = block_lookup(block, node);
		if ((!name && extern_node_matches(finddata->name, node)) ||
		    name == finddata->name) {
			struct occurrence *occurrence =
				xmalloc(sizeof(struct occurrence));
			occurrence->start = node->start;
			occurrence->end = node->end;
			occurrence->cfile = finddata->cfile;
			occurrence->next = finddata->occurrence;
			finddata->occurrence = occurrence;
		}
	}
	/* don't check nodes outside defining block */
	return 1;
}

static struct occurrence *reverse(struct occurrence *o)
{
	struct occurrence *newhead = NULL;
	struct occurrence *cur = o;
	while (cur) {
		struct occurrence *next = cur->next;
		cur->next = newhead;
		newhead = cur;
		cur = next;
	}
	return newhead;
}

struct occurrence *find_name(struct project *project, struct name *name)
{
	if (name) {
		struct finddata finddata;
		int i;
		finddata.occurrence = NULL;
		finddata.name = name;
		for (i = project->count - 1; i >= 0; --i) {
			struct cfile *cur = project->files[i];
			finddata.block = cur->block;
			finddata.cfile = cur;
			node_walk(cur->node, does_match, &finddata);
		}
		return reverse(finddata.occurrence);
	}
	return NULL;
}

void free_occurrences(struct occurrence *occurrence_list)
{
	struct occurrence *cur = occurrence_list;
	while (cur) {
		struct occurrence *next = cur->next;
		free(cur);
		cur = next;
	}
}
