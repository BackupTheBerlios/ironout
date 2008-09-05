#include <string.h>
#include "path.h"

void dirname(char *dir, char *path)
{
	char *slash = strrchr(path, '/');
	if (slash) {
		size_t len = slash - path + 1;
		memcpy(dir, path, len);
		dir[len] = '\0';
	} else {
		strcpy(dir, "./");
	}
}

void basename(char *name, char *path)
{
	char *slash = strrchr(path, '/');
	if (slash)
		strcpy(name, slash + 1);
	else
		strcpy(name, path);
}
