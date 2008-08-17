#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "strutils.h"

#define MAXLINELEN	1024
#define MAXPATHLEN	1024
#define TEMPDIR		"ironproj"


struct input {
	FILE *input;
	char buf[MAXLINELEN];
};

static struct input *input_open(char *filename)
{
	FILE *input = fopen(filename, "r");
	struct input *result;
	if (!input)
		return NULL;
	result = malloc(sizeof(struct input));
	result->input = input;
	result->buf[0] = '\0';
	return result;
}

static char *input_line(struct input *input)
{
	if (*input->buf)
		return input->buf;
	if (!fgets(input->buf, MAXLINELEN, input->input))
		return NULL;
	return input->buf;
}

static void input_next(struct input *input)
{
	input->buf[0] = '\0';
}

static void input_free(struct input *input)
{
	if (input->input)
		fclose(input->input);
	free(input);
}

static int write_file(struct input *input)
{
	char separator[128];
	char filename[MAXPATHLEN];
	FILE *output;
	char *line = input_line(input);
	nthtoken(separator, line, " \n", 1);
	nthtoken(filename, line, " \n", 3);
	output = fopen(filename, "w");
	input_next(input);
	while ((line = input_line(input))) {
		if (startswith(line, separator))
			break;
		fputs(line, output);
		input_next(input);
	}
	fclose(output);
	return 0;
}

static int match_files(struct input *input, FILE *realfile)
{
	char separator[128];
	char line[MAXLINELEN];
	char *expected = input_line(input);

	nthtoken(separator, expected, " \n", 1);
	input_next(input);
	while ((expected = input_line(input))) {
		if (startswith(expected, separator))
			break;
		if (!fgets(line, MAXLINELEN, realfile))
			return 1;
		if (strcmp(expected, line))
			return 1;
		input_next(input);
	}
	if (fgets(line, MAXLINELEN, realfile))
		return 1;
	return 0;
}

static int read_file(struct input *input)
{
	char filename[MAXPATHLEN];
	FILE *realinput;
	char *line = input_line(input);
	int result;

	nthtoken(filename, line, " \n", 3);
	if (!(realinput = fopen(filename, "r")))
		return 1;
	result = match_files(input, realinput);
	fclose(realinput);
	return result;
}

static int exec_ironout(struct input *input, char *ironout)
{
	char token[128];
	char command[MAXPATHLEN];
	FILE *output;
	char *line = input_line(input);
	int result;
	char *cur = command;

	line = nthtoken(token, line, " \n", 2);
	strcpy(cur, ironout);
	cur += strlen(cur);
	while (*line) {
		line = readtoken(token, line, " \n");
		cur += sprintf(cur, " %s", token);
	}
	if (!(output = popen(command, "r")))
		return 1;
	result = match_files(input, output);
	pclose(output);
	return result;
}

static int read_comment(struct input *input)
{
	char separator[128];
	char *line = input_line(input);
	line = readtoken(separator, line, " \n");
	input_next(input);
	while ((line = input_line(input)) && !startswith(line, separator))
		input_next(input);
	return 0;
}

static int runtest(char *filename, char *ironout)
{
	struct input *input = input_open(filename);
	char current_line[MAXLINELEN];
	char cmd[128];
	char *line;
	if (!input)
		return 1;
	while ((line = input_line(input))) {
		int result = -1;
		strcpy(current_line, line);
		nthtoken(cmd, line, " \n", 2);
		if (!strcmp(cmd, "comment") || !strcmp(cmd, "#") || !*cmd)
			result = read_comment(input);
		if (!strcmp(cmd, "write") || !strcmp(cmd, ">"))
			result = write_file(input);
		if (!strcmp(cmd, "read") || !strcmp(cmd, "<"))
			result = read_file(input);
		if (!strcmp(cmd, "ironout"))
			result = exec_ironout(input, ironout);
		if (result == -1) {
			printf("unknown cmd: %s\n", cmd);
			return 1;
		}
		if (result > 0) {
			char *testname = filename;
			if (strchr(testname, '/'))
				testname = strrchr(testname, '/') + 1;
			printf("%s failed%s", testname, current_line);
			return 1;
		}
	}
	input_free(input);
	return 0;
}

static void mktempdir(char *path)
{
	mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR);
}

static void rmtempdir(char *path)
{
	struct dirent *ent;
	DIR *dir = opendir(path);
	while ((ent = readdir(dir))) {
		char child[MAXPATHLEN];
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		sprintf(child, "%s/%s", path, ent->d_name);
		if (ent->d_type == DT_DIR)
			rmtempdir(child);
		else
			unlink(child);
	}
	closedir(dir);
	rmdir(path);
}

int runtests(char *dirname, char *ironout)
{
	struct dirent *ent;
	DIR *dir;
	int total = 0;
	int fails = 0;
	dir = opendir(dirname);
	if (!dir) {
		printf("failed: %s does not exist\n", dirname);
		return 1;
	}
	while ((ent = readdir(dir))) {
		char *name = ent->d_name;
		if (startswith(name, "test-") && !endswith(name, "~")) {
			char path[MAXPATHLEN];
			sprintf(path, "%s/%s", dirname, name);
			total++;
			if (runtest(path, ironout)) {
				fails++;
			}
		}
	}
	closedir(dir);
	if (fails)
		printf("\n%d of %d failed\n", fails, total);
	else
		printf("%d succeeded\n", total);
	return fails;
}

static void abspath(char *path, char *rel)
{
	if (rel[0] != '/') {
		getcwd(path, MAXPATHLEN);
		strcat(path, "/");
		strcat(path, rel);
	} else {
		strcpy(path, rel);
	}
}

int main(int argc, char **argv)
{
	int result;
	char testdir[MAXPATHLEN];
	char ironout[MAXPATHLEN];
	char origin[MAXPATHLEN];
	if (argc < 2) {
		printf("Usage: %s testdir\n", argv[0]);
		return 1;
	}
	mktempdir(TEMPDIR);
	getcwd(origin, MAXPATHLEN);
	abspath(testdir, argv[1]);
	abspath(ironout, "ironout");
	chdir(TEMPDIR);

	result = runtests(testdir, ironout);

	chdir(origin);
	rmtempdir(TEMPDIR);
	return result;
}
