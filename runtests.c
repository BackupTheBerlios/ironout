#include <stdio.h>
#include <dirent.h>
#include "strutils.h"


static int runtest()
{
	return 0;
}

int runtests(char *dirname)
{
	struct dirent *ent;
	DIR *dir;
	int total = 0;
	int fails = 0;
	dir = opendir(dirname);
	while ((ent = readdir(dir))) {
		char *name = ent->d_name;
		if (startswith(name, "t") && !endswith(name, "~")) {
			char path[1024];
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
