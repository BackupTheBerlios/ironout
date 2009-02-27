/*
 * ironout  - A C refactoring tool
 *
 * Copyright (C) 2008 Ali Gholami Rudi
 *
 */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "block.h"
#include "cfile.h"
#include "find.h"
#include "hash.h"
#include "name.h"
#include "path.h"
#include "project.h"
#include "rename.h"

#define MAXPATHLEN	1024

static void require_args(int argc, char **argv, int required, char *usage)
{
	if (argc < required + 1) {
		fprintf(stderr, "Usage: %s %s\n\n", argv[0], usage);
		fprintf(stderr, "%s command requires at least %d arguments\n",
			argv[0], required);
		exit(1);
	}
}

static int parse_cmd(int argc, char **argv)
{
	struct cfile *cfile;
	require_args(argc, argv, 1, "FILENAME");
	cfile = cfile_init(argv[1]);
	if (cfile)
		cfile_free(cfile);
	return !cfile;
}

static int getname_cmd(int argc, char **argv)
{
	struct cfile *cfile;
	struct node *found;
	require_args(argc, argv, 2, "FILENAME OFFSET");
	cfile = cfile_init(argv[1]);
	found = node_find(cfile->node, atoi(argv[2]));
	if (found && (found->type == AST_IDENTIFIER ||
		      found->type == AST_TYPENAME))
		puts(found->data);
	cfile_free(cfile);
	return 0;
}

static int find_cmd(int argc, char **argv)
{
	char dir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct project *project;
	struct cfile *cfile;
	struct hit *hits, *cur;
	struct name *name;
	require_args(argc, argv, 2, "FILENAME OFFSET");
	dirname(dir, argv[1]);
	basename(filename, argv[1]);
	chdir(dir);
	project = project_init(".");
	cfile = project_find(project, filename);
	if (!cfile)
		return 1;
	name = name_find(project, cfile, argv[2]);
	hits = find_name(project, name);
	cur = hits;
	while (cur) {
		printf("%s %ld %ld\n", cur->cfile->name, cur->start, cur->end);
		cur = cur->next;
	}
	free_hits(hits);
	project_free(project);
	return 0;
}

static int rename_cmd(int argc, char **argv)
{
	char dir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct project *project;
	struct cfile *cfile;
	struct name *name;
	require_args(argc, argv, 3, "FILENAME OFFSET NEWNAME");
	dirname(dir, argv[1]);
	basename(filename, argv[1]);
	chdir(dir);
	project = project_init(".");
	cfile = project_find(project, filename);
	if (!cfile)
		return 1;
	name = name_find(project, cfile, argv[2]);
	rename_name(project, name, argv[3]);
	project_free(project);
	return 0;
}

static char *usage =
	"Usage: %s COMMAND [ARGS]\n\n"
	"Command can be:\n"
	"  rename:   rename a name and report the changes as a patch\n"
	"  find:     find and list occurrences of a name\n"
	"  getname:  print the name under point\n"
	"  version:  print version number and exit\n";

static void print_help(int argc, char **argv)
{
	printf(usage, argv[0]);
}

static char *version =
	"Ironout 0.0.2\n"
	"Copyright (C) 2008 Ali Gholami Rudi\n"
	"You can redistribute and/or modify Ironout under the terms\n"
	"of GNU General Public License.  For more information, see\n"
	"the file named COPYING.\n";


static void print_version(int argc, char **argv)
{
	printf(version);
}

int main(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "vh")) != -1) {
		switch (c) {
		case 'v':
			print_version(argc, argv);
			return 0;
		case 'h':
			print_help(argc, argv);
			return 0;
		case '?':
		default:
			fprintf(stderr, "Bad command line option\n");
			return 1;
		}
	}
	if (argc > optind) {
		char *command = argv[optind];
		char **nargv = argv + optind;
		int nargc = argc - optind;
		if (!strcmp(command, "parse"))
			return parse_cmd(nargc, nargv);
		if (!strcmp(command, "getname"))
			return getname_cmd(nargc, nargv);
		if (!strcmp(command, "find"))
			return find_cmd(nargc, nargv);
		if (!strcmp(command, "rename"))
			return rename_cmd(nargc, nargv);
		if (!strcmp(command, "version")) {
			print_version(argc, argv);
			return 0;
		}
	}
	print_help(argc, argv);
	return 1;
}
