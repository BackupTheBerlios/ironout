#include <stdlib.h>
#include "name.h"
#include "utils.h"

struct name *name_init(char *name, int flags)
{
	struct name *result = xmalloc(sizeof(struct name));
	result->name = name;
	result->flags = flags;
	return result;
}

void name_free(struct name *name)
{
	free(name);
}
