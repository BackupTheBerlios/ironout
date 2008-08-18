#include <stdio.h>
#include <stdlib.h>


void *xmalloc(size_t size)
{
	void *value = malloc(size);
	if (!value) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return value;
}
