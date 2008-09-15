#ifndef _FIND_H
#define _FIND_H

#include "cfile.h"
#include "project.h"

struct hit {
	long start;
	long end;
	struct cfile *cfile;
	struct hit *next;
};

struct hit *find_name(struct project *project, struct name *name);
void free_hits(struct hit *hits);

#endif
