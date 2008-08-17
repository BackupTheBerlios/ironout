#include <stdlib.h>
#include <string.h>
#include "hash.h"


struct hash *hash_init(long (*datahash) (void *data),
		       long (*keyhash) (void *data),
		       int (*datacmp) (void *data, void *key),
		       int size)
{
	struct hash *hash = malloc(sizeof(struct hash));
	hash->datahash = datahash;
	hash->keyhash = keyhash;
	hash->datacmp = datacmp;
	hash->size = size;
	hash->collisions = 0;
	hash->table = malloc(size * sizeof(struct entry *));
	memset(hash->table, 0, size * sizeof(struct entry *));
	return hash;
}

void hash_release(struct hash *hash)
{
	int i;
	for (i = 0; i < hash->size; i++) {
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
	return (value >= 0 ? value : -value) % hash->size;
}

static void hash_grow(struct hash *hash, int size);

void hash_put(struct hash *hash, void *data)
{
	long i = hash_key(hash, hash->datahash(data));
	struct entry *newdata = malloc(sizeof(struct entry *));
	newdata->data = data;
	if (hash->table[i])
		hash->collisions++;
	newdata->next = hash->table[i];
	hash->table[i] = newdata;
	if (hash->collisions > hash->size / 2)
		hash_grow(hash, hash->size * 2);
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

static void hash_grow(struct hash *hash, int size)
{
	struct hash *newhash = hash_init(hash->datahash, hash->keyhash,
					 hash->datacmp, size);
	int i;
	for (i = 0; i < hash->size; i++) {
		struct entry *cur = hash->table[i];
		while (cur) {
			struct entry *old = cur;
			hash_put(newhash, cur->data);
			cur = cur->next;
			free(old);
		}
	}
	free(hash->table);
	memcpy(hash, newhash, sizeof(*hash));
	free(newhash);
}
