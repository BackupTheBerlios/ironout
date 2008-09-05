#ifndef _PROJECT_H
#define _PROJECT_H

#include "cfile.h"

struct project {
	struct cfile **files;
	int count;
};

struct project *project_init();
void project_free(struct project *project);

struct cfile *project_find(struct project *project, char *filename);

#endif
