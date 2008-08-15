#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "parse") && argc > 2) {
			free_node(parse(argv[2]));
			return 0;
		}
		if (!strcmp(argv[1], "findat") && argc > 3) {
			struct node *node = parse(argv[2]);
			node_at(node, atoi(argv[3]));
			free_node(node);
			return 0;
		}
	}
	printf("Usage: %s COMMAND [ARGS]\n", argv[0]);
	return 1;
}
