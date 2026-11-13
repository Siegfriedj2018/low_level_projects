#define _POSIX_C_SOURCE 200809L
#include <kv.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define TOMBSTONE 0x1

size_t hash( char *val, int capacity) {
	size_t hash = 0x13371337deadbeef;

	while (*val) {
		hash ^= *val;
		hash = hash << 8;
		hash += *val;

		val++;
	}

	return hash % capacity;
}

/* fn kv_put
 * params:
 *  - db: a pointer to the db
 *  - key: a pointer to the key value
 *  - value: a pointer to the value itself
 * returns: the index of the key, otherwise on error returns -1, or db occupied -2
 */
int kv_put(kv_t *db, char *key, char *value) {
	if (!db || !key || !value) return -1;

	size_t idx = hash(key, db->capacity);
	ssize_t first_tombstone = -1;

	for (size_t i = 0; i < db->capacity; i++) {
		size_t real_idx = (idx + i) % db->capacity;
		kv_entry_t *entry = &db->entries[real_idx];

		// Empty slot: key does not exist further down the chain
		if (!entry->key) {
			size_t target_idx = (first_tombstone != -1) ? (size_t)first_tombstone : real_idx;
			kv_entry_t *target = &db->entries[target_idx];

			char *newval = strdup(value);
			char *newkey = strdup(key);
			if (!newval || !newkey) {
				free(newkey);
				free(newval);
				return -1;
			}
			target->key = newkey;
			target->value = newval;
			db->count++;
			return target_idx;
		}

		// Tombstone slot: record the first one seen and keep probing
		if (entry->key == (void*)TOMBSTONE) {
			if (first_tombstone == -1) {
				first_tombstone = (ssize_t)real_idx;
			}
			continue;
		}

		// Key already exists: update value in place
		if (!strcmp(entry->key, key)) {
			char *newval = strdup(value);
			if (!newval) return -1;
			free(entry->value);
			entry->value = newval;
			return real_idx;
		}
	}

	// Probed entire table without finding an empty slot or matching key,
	// but saw a tombstone we can reuse
	if (first_tombstone != -1) {
		kv_entry_t *target = &db->entries[first_tombstone];
		char *newval = strdup(value);
		char *newkey = strdup(key);
		if (!newval || !newkey) {
			free(newkey);
			free(newval);
			return -1;
		}
		target->key = newkey;
		target->value = newval;
		db->count++;
		return first_tombstone;
	}

	// Table is completely full
	return -2;
}

kv_t *kv_init(size_t capacity) {
	if (capacity == 0) {
		return NULL;
	}

	kv_t *table = malloc(sizeof(kv_t));
	if (table == NULL) {
		return NULL;
	}

	table->capacity = capacity;
	table->count = 0;

	table->entries = calloc(sizeof(kv_entry_t), capacity);
	if (table->entries == NULL) {
		free(table);
		return NULL;
	}

	return table;
}
