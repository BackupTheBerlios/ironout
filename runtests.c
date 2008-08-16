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

static void write_file(struct input *input)
{
	char separator[128];
	char filename[MAXPATHLEN];
	FILE *output;
	char *line = input_line(input);
	line = readtoken(separator, line, " ");
	line = readtoken(filename, line, " ");
	line = readtoken(filename, line, " \n");
	output = fopen(filename, "w");
	input_next(input);
	while ((line = input_line(input))) {
		if (startswith(line, separator))
			break;
		fputs(line, output);
		input_next(input);
	}
	fclose(output);
}

static int runtest(char *filename)
{
	struct input *input = input_open(filename);
	char command[128];
	char *line;
	if (!input)
		return 1;
	while ((line = input_line(input))) {
		line = readtoken(command, line, " \n");
		line = readtoken(command, line, " \n");
		if (!strcmp(command, "write")) {
			write_file(input);
			continue;
		}
		printf("unknown command: %s\n", command);
		return 1;
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
				printf("* %s failed\n", path);
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
