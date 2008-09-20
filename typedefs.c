#include <stdlib.h>
#include <string.h>
#include "hash.h"

static struct hash *typedefs = NULL;

static int str_cmp(void *data, void *key)
{
	return strcmp(data, key);
}

static void add_type(char *typename)
{
	if (!typedefs)
		typedefs = hash_init(str_hash, str_hash, str_cmp, 16);
	hash_put(typedefs, typename);
}

int typedef_name(char *filename, char *name, long offset)
{
	if (!typedefs) {
		add_type("FILE");
		add_type("size_t");
		add_type("DIR");
	}
	return hash_get(typedefs, name) != NULL;
}
