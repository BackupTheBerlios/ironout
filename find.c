#include <stdlib.h>
#include <string.h>
#include "find.h"
#include "utils.h"


struct finddata {
	struct cfile *cfile;
	struct block *block;
	struct name *name;
	struct hit *hit;
};

static int extern_node_matches(struct name *expected, struct node *node)
{
	int flags = modifier_flags(node);
	if (!modifiers_match(expected, flags) || expected->flags & NAME_STATIC)
		return 0;
	return !(flags & (NAME_FIELD | NAME_PARAMDECL));
}

static int does_match(struct node *node, void *data)
{
	struct finddata *finddata = data;
	if ((node->type == AST_IDENTIFIER || node->type == AST_TYPENAME) &&
	    !strcmp(finddata->name->name, node->data)) {
		struct block *block = block_find(finddata->block, node->start);
		struct name *name = block_lookup(block, node);
		if ((!name && extern_node_matches(finddata->name, node)) ||
		    name == finddata->name) {
			struct hit *hit = xmalloc(sizeof(struct hit));
			hit->start = node->start;
			hit->end = node->end;
			hit->cfile = finddata->cfile;
			hit->next = finddata->hit;
			finddata->hit = hit;
		}
	}
	/* don't check nodes outside defining block */
	return 1;
}

static struct hit *reverse(struct hit *o)
{
	struct hit *newhead = NULL;
	struct hit *cur = o;
	while (cur) {
		struct hit *next = cur->next;
		cur->next = newhead;
		newhead = cur;
		cur = next;
	}
	return newhead;
}

struct hit *find_name(struct project *project, struct name *name)
{
	if (name) {
		struct finddata finddata;
		int i;
		finddata.hit = NULL;
		finddata.name = name;
		for (i = project->count - 1; i >= 0; --i) {
			struct cfile *cur = project->files[i];
			finddata.block = cur->block;
			finddata.cfile = cur;
			node_walk(cur->node, does_match, &finddata);
		}
		return reverse(finddata.hit);
	}
	return NULL;
}

void free_hits(struct hit *hit_list)
{
	struct hit *cur = hit_list;
	while (cur) {
		struct hit *next = cur->next;
		free(cur);
		cur = next;
	}
}
