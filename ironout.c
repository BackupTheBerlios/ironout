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

static int find_cmd(char *path, long offset)
{
	char dir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct project *project;
	struct cfile *cfile;
	struct occurrence *occurrences, *cur;
	dirname(dir, path);
	basename(filename, path);
	chdir(dir);
	project = project_init(".");
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
	project_free(project);
	return 0;
}

static int rename_cmd(char *path, long offset, char *newname)
{
	char dir[MAXPATHLEN];
	char filename[MAXPATHLEN];
	struct project *project;
	struct cfile *cfile;
	struct occurrence *occurrences;
	dirname(dir, path);
	basename(filename, path);
	chdir(dir);
	project = project_init(".");
	cfile = project_find(project, filename);
	if (!cfile)
		return 1;
	occurrences = find_at(project, cfile, offset);
	rename_at(occurrences, newname);

	free_occurrences(occurrences);
	project_free(project);
	return 0;
}

static void print_help(int argc, char **argv)
{
		printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
}

static void print_version(int argc, char **argv)
{
}

int main(int argc, char **argv)
{
	int c;
	while ((c = getopt (argc, argv, "vh")) != -1) {
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
		if (!strcmp(command, "parse") && argc > 2)
			return parse_cmd(argv[2]);
		if (!strcmp(command, "getname") && argc > 3)
			return getname_cmd(argv[2], atoi(argv[3]));
		if (!strcmp(command, "find") && argc > 3)
			return find_cmd(argv[2], atoi(argv[3]));
		if (!strcmp(command, "rename") && argc > 4)
			return rename_cmd(argv[2], atoi(argv[3]), argv[4]);
	}
	print_help(argc, argv);
	return 1;
}
