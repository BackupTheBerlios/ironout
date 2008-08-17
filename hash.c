#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define HASHSIZE	128


struct hash *hash_init(long (*datahash) (void *data),
		       long (*keyhash) (void *data),
		       int (*datacmp) (void *data, void *key))
{
	struct hash *hash = malloc(sizeof(struct hash));
	hash->datahash = datahash;
	hash->keyhash = keyhash;
	hash->datacmp = datacmp;
	hash->table = malloc(HASHSIZE * sizeof(struct entry *));
	memset(hash->table, 0, HASHSIZE * sizeof(struct entry *));
	return hash;
}

void hash_release(struct hash *hash)
{
	int i;
	for (i = 0; i < HASHSIZE; i++) {
		struct entry *cur = hash->table[i];
		while (cur) {
			struct entry *old = cur;
			cur = cur->next;
			free(old);
		}
	}
	free(hash->table);
	free(hash);
}

long str_hash(char *s)
{
	long x = *s << 7;
	while (*s)
		x = (1000003 * x) ^ *s++;
	return x;
}

static long hash_key(struct hash *hash, long value)
{
	return (value >= 0 ? value : -value) % HASHSIZE;
}

void hash_put(struct hash *hash, void *data)
{
	long i = hash_key(hash, hash->datahash(data));
	struct entry *newdata = malloc(sizeof(struct entry *));
	newdata->data = data;
	newdata->next = hash->table[i];
	hash->table[i] = newdata;
}

void *hash_get(struct hash *hash, void *key)
{
	struct entry *cur = hash->table[hash_key(hash, hash->keyhash(key))];
	while (cur) {
		if (!hash->datacmp(cur->data, key))
			return cur->data;
		cur = cur->next;
	}
	return NULL;
}
