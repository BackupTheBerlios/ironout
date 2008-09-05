#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "strutils.h"
#include "utils.h"

#define MAXFILES	128

static int cfile_cmp(void *o1, void *o2)
{
	struct cfile *c1 = *(struct cfile **) o1;
	struct cfile *c2 = *(struct cfile **) o2;
	return -strcmp(c1->name, c2->name);
}

struct project *project_init(char *root)
{
	struct project *project;
	struct dirent *dirent;
	DIR *dir = opendir(root);
	project = xmalloc(sizeof(*project));
	project->files = xmalloc(sizeof(*project->files) * MAXFILES);
	project->count = 0;
	while ((dirent = readdir(dir))) {
		struct cfile *cfile;
		char *name = dirent->d_name;
		if (startswith(name, "."))
			continue;
		if (endswith(name, ".h") || endswith(name, ".c")) {
			cfile = cfile_init(name);
			if (name)
				project->files[project->count++] = cfile;
		}
	}
	qsort(project->files, project->count,
	      sizeof(*project->files), cfile_cmp);
	return project;
}

void project_free(struct project *project)
{
	free(project->files);
	free(project);
}

struct cfile *project_find(struct project *project, char *filename)
{
	int i;
	for (i = 0; i < project->count; i++)
		if (!strcmp(project->files[i]->name, filename))
			return project->files[i];
	return NULL;
}
