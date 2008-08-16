#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutils.h"


#define MAXLINELEN	1024
#define MAXPATHLEN	1024


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

static int read_file(struct input *input)
{
	char separator[128];
	char filename[MAXPATHLEN];
	FILE *realinput;
	char *line = input_line(input);
	char buf[MAXLINELEN];

	nthtoken(separator, line, " \n", 1);
	nthtoken(filename, line, " \n", 3);

	if (!(realinput = fopen(filename, "r")))
		return 1;
	input_next(input);
	while ((line = input_line(input))) {
		if (startswith(line, separator))
			break;
		if (!fgets(buf, MAXLINELEN, realinput))
			return 1;
		if (strcmp(line, buf))
			return 1;
		input_next(input);
	}
	if (fgets(buf, MAXLINELEN, realinput))
		return 1;
	fclose(realinput);
	return 0;
}

static int read_comment(struct input *input)
{
	char separator[128];
	char *line = input_line(input);
	line = readtoken(separator, line, " \n");
	input_next(input);
	while ((line = input_line(input)) && !startswith(line, separator)) {
		input_next(input);
	}
	return 0;
}

static int runtest(char *filename)
{
	struct input *input = input_open(filename);
	char command[128];
	char *line;
	if (!input)
		return 1;
	while ((line = input_line(input))) {
		int result = -1;
		nthtoken(command, line, " \n", 2);
		if (!strcmp(command, "comment"))
			result = read_comment(input);
		if (!strcmp(command, "write"))
			result = write_file(input);
		if (!strcmp(command, "read"))
			result = read_file(input);
		if (result == -1) {
			printf("unknown command: %s\n", command);
			return 1;
		}
		if (result != 0) {
			printf("failed: %s\n", line);
			return 1;
		}
	}
	input_free(input);
	return 0;
}

int runtests(char *dirname)
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
		if (startswith(name, "t") && !endswith(name, "~")) {
			char path[MAXPATHLEN];
			sprintf(path, "%s/%s", dirname, name);
			total++;
			if (runtest(path)) {
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

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s testdir\n", argv[0]);
		return 1;
	}
	return runtests(argv[1]);
}
