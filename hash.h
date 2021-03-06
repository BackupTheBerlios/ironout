#ifndef _HASH_H
#define _HASH_H

struct entry {
	struct entry *next;
	void *data;
};

struct hash {
	struct entry **table;
	long (*datahash) (void *data);
	long (*keyhash) (void *data);
	int (*datacmp) (void *data, void *key);
	int size;
	int collisions;
};

struct hash *hash_init(long (*datahash) (void *data),
		       long (*keyhash) (void *data),
		       int (*datacmp) (void *data, void *key),
		       int size);
void hash_release(struct hash *hash);

void hash_put(struct hash *hash, void *value);
void *hash_get(struct hash *hash, void *key);
void hash_walk(struct hash *hash,
	       void (*walk)(void *data, void *arg),
	       void *arg);

long str_hash(void *s);

#endif
