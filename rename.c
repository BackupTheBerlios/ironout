#include <string.h>
#include "find.h"
#include "rename.h"
#include "src.h"

void rename_name(struct project *project, struct name *name, char *newname)
{
	struct occurrence *all = find_name(project, name);
	struct occurrence *cur = all;
	while (cur) {
		struct cfile *cfile = cur->cfile;
		struct src *orig = src_from_file(cur->cfile->name);
		struct src *changed = src_copy(orig);
		long diffs = 0;
		while (cur && cur->cfile == cfile) {
			long start = cur->start + diffs;
			long end = cur->end + diffs;
			src_delete(changed, start, end);
			src_insert(changed, start, newname);
			cur = cur->next;
			diffs += strlen(newname) - (end - start);
		}
		src_print_diffs(orig, changed, cfile->name);
		src_free(orig);
		src_free(changed);
	}
	free_occurrences(all);
}
