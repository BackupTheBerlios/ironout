#include <stdlib.h>
#include "find.h"
#include "utils.h"


struct finddata {
	struct cfile *cfile;
	struct block *block;
	struct name *name;
	struct occurrence *occurrence;
};

static int checknode(struct node *node, void *data)
{
	struct finddata *finddata = data;
	if (node->type == AST_IDENTIFIER || node->type == AST_TYPENAME) {
		struct block *block = block_find(finddata->block, node->start);
		if (block_lookup(block, node) == finddata->name) {
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

struct occurrence *find_at(struct cfile *cfile, long offset)
{
	struct node *node = node_find(cfile->node, offset);
	struct block *block = block_find(cfile->block, offset);
	struct name *name = block_lookup(block, node);
	if (name) {
		struct block *defblock = block_defining(block, node);
		if (defblock) {
			struct finddata finddata;
			finddata.block = defblock;
			finddata.name = name;
			finddata.cfile = cfile;
			finddata.occurrence = NULL;
			node_walk(cfile->node, checknode, &finddata);
			return reverse(finddata.occurrence);
		}
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
