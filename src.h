#ifndef _SRC_H
#define _SRC_H

struct src {
	int count;
	char **lines;
	long *offsets;
	/* the index from which offsets array is invalid */
	int invalid_from;
};

struct src *src_from_file(char *filename);
struct src *src_copy(struct src *src);
void src_free(struct src *src);

void src_delete(struct src *src, long from, long to);
void src_insert(struct src *src, long offset, char *s);

void src_print_diffs(struct src *src1, struct src *src2, char *filename);

#endif
