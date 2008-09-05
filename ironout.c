#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "block.h"
#include "cfile.h"
#include "find.h"
#include "hash.h"
#include "name.h"
#include "project.h"


static int parse_cmd(char *filename)
{
	struct cfile *cfile = cfile_init(filename);
	if (cfile)
		cfile_free(cfile);
	return !!cfile;
}

static int getname_cmd(char *filename, long offset)
{
	struct cfile *cfile = cfile_init(filename);
	struct node *found = node_find(cfile->node, offset);
	if (found && (found->type == AST_IDENTIFIER ||
		      found->type == AST_TYPENAME))
		puts(found->data);
	cfile_free(cfile);
	return 0;
}

static int find_cmd(char *filename, long offset)
{
	struct project *project = project_init();
	struct cfile *cfile;
	struct occurrence *occurrences, *cur;
	cfile = project_find(project, filename);
	if (!cfile)
		return 1;
	occurrences = find_at(project, cfile, offset);
	cur = occurrences;
	while (cur) {
		printf("%s %ld %ld\n", cur->cfile->name, cur->start, cur->end);
		cur = cur->next;
	}
	free_occurrences(occurrences);
	cfile_free(cfile);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "parse") && argc > 2)
			return parse_cmd(argv[2]);
		if (!strcmp(argv[1], "getname") && argc > 3)
			return getname_cmd(argv[2], atoi(argv[3]));
		if (!strcmp(argv[1], "find") && argc > 3)
			return find_cmd(argv[2], atoi(argv[3]));
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
