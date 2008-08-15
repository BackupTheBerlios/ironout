#include <stdio.h>
#include <string.h>
#include "parse.h"


int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "parse") && argc > 2) {
			free_node(parse(argv[2]));
			return 0;
		}
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
