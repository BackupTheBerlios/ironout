#include <stdio.h>
#include <string.h>
#include "parse.h"


int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "parse") && argc > 2)
			return parse(argv[2]);
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
}
