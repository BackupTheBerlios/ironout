#include <string.h>
#include "src.h"
#include "rename.h"
#include "src.h"

void rename_at(struct occurrence *occurrences, char *replacement)
{
	struct occurrence *cur = occurrences;
	while (cur) {
		struct cfile *cfile = cur->cfile;
		struct src *orig = src_from_file(cur->cfile->name);
		struct src *changed = src_copy(orig);
		long diffs = 0;
		while (cur && cur->cfile == cfile) {
			long start = cur->start + diffs;
			long end = cur->end + diffs;
			src_delete(changed, start, end);
			src_insert(changed, start, replacement);
			cur = cur->next;
			diffs += strlen(replacement) - (end - start);
		}
		src_print_diffs(orig, changed, cfile->name);
		src_free(orig);
		src_free(changed);
	}
}
