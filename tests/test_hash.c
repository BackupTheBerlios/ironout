#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"


static void equal(void *p1, void *p2, char *msg)
{
	if (p1 != p2) {
		fprintf(stderr, "<%s> failed!\n", msg);
		exit(1);
	}
}

static long _strhash(void *data)
{
	return ((char *) data)[0];
}

static int _strcmp(void *data, void *key)
{
	return strcmp(data, key);
}

static long _inthash(void *data)
{
	return *((int *) data);
}

static int _intcmp(void *data, void *key)
{
	int i1 = *((int *) data);
	int i2 = *((int *) key);
	return i1 - i2;
}

static void test_inthash()
{
	struct hash *hash = hash_init(_inthash, _inthash, _intcmp);
	int a = 0, b = 1;
	int zero = 0, one = 1;

	/* NULL should be returned if a key is missing */
	equal(NULL, hash_get(hash, &one), "0");

	/* basic insertion */
	hash_put(hash, &a);
	equal(&a, hash_get(hash, &zero), "1");

	/* putting more values */
	hash_put(hash, &b);
	equal(&a, hash_get(hash, &zero), "2");
	equal(&b, hash_get(hash, &one), "3");

	hash_release(hash);
}

static void test_strhash()
{
	struct hash *hash = hash_init(_strhash, _strhash, _strcmp);
	char *a = "a";
	char *b = "b";

	/* NULL should be returned if a key is missing */
	equal(NULL, hash_get(hash, a), "10");

	/* basic insertion */
	hash_put(hash, a);
	equal(a, hash_get(hash, "a"), "11");

	/* putting more values */
	hash_put(hash, b);
	equal(a, hash_get(hash, "a"), "12");
	equal(b, hash_get(hash, "b"), "13");

	hash_release(hash);
}

int main()
{
	test_inthash();
	test_strhash();
	return 0;
}
