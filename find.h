#ifndef _FIND_H
#define _FIND_H

#include "cfile.h"

struct occurrence {
	long start;
	long end;
	struct cfile *cfile;
	struct occurrence *next;
};

struct occurrence *find_at(struct cfile *cfile, long offset);
void free_occurrences(struct occurrence *occurrences);

#endif
