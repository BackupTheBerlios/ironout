#ifndef _FIND_H
#define _FIND_H

#include "cfile.h"
#include "project.h"

struct occurrence {
	long start;
	long end;
	struct cfile *cfile;
	struct occurrence *next;
};

struct occurrence *find_at(struct project *project,
			   struct cfile *cfile, long offset);
void free_occurrences(struct occurrence *occurrences);

#endif
