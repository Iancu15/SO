//
// Created by alex on 11.03.2022.
//

#include "hashmap.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HashMap *empty_hashmap(int capacity)
{
	HashMap *hashmap;
	int i;

	hashmap = malloc(sizeof(HashMap));
	if (!hashmap)
		return NULL;

	hashmap->size = 0;
	hashmap->capacity = capacity;
	hashmap->map = malloc(capacity * sizeof(HashMapEntry *));
	if (!hashmap->map)
		return NULL;

	for (i = 0; i < capacity; i++)
		hashmap->map[i] = NULL;

	return hashmap;
}

/**
 * frees an entry and all its following entries in the list
 */
static void free_hashmap_entry(HashMapEntry *entry)
{
	if (entry->next != NULL)
		free_hashmap_entry(entry->next);

	free(entry->key);
	free(entry->value);
	free(entry);
}

/**
 * frees the map associated with the hashmap struct
 */
static void free_map(HashMapEntry **map, int capacity)
{
	int i;

	for (i = 0; i < capacity; i++) {
		if (map[i] != NULL)
			free_hashmap_entry(map[i]);
	}

	free(map);
}

void free_hashmap(HashMap *hashmap)
{
	free_map(hashmap->map, hashmap->capacity);
	free(hashmap);
}

/**
 * generates the hashcode based on the key
 */
static int hash_code(char *key, int mod)
{
	int hashcode, len, i, product;

	hashcode = 0;
	len = strlen(key);
	for (i = 0; i < len; i++) {
		product = ((23 % mod) * (hashcode % mod)) % mod;
		hashcode = ((product % mod) + (key[i] % mod)) % mod;
	}

	return hashcode % mod;
}

/**
 * get the index in the map associated with hash of the key
 */
static int get_index(HashMap *hashmap, char *key)
{
	return hash_code(key, hashmap->capacity);
}

/**
 * gets the entry associated with the key
 * return NULL if none is associated
 */
static HashMapEntry *get_entry(HashMap *hashmap, char *key)
{
	int index;
	HashMapEntry *entry;

	index = get_index(hashmap, key);
	entry = hashmap->map[index];
	while (entry != NULL && strcmp(entry->key, key) != 0)
		entry = entry->next;

	return entry;
}

/**
 * doubles the capacity of the hashmap,
 * reallocates the memory and rehashes the entire map
 */
static int rehash(HashMap *hashmap)
{
	HashMapEntry **new_map, **old_map;
	HashMapEntry *entry;
	int i;

	hashmap->capacity *= 2;
	new_map = malloc(hashmap->capacity * sizeof(HashMapEntry *));
	if (!new_map)
		return 12;

	old_map = hashmap->map;
	hashmap->map = new_map;

	for (i = 0; i < hashmap->capacity; i++)
		hashmap->map[i] = NULL;

	for (i = 0; i < hashmap->capacity / 2; i++) {
		entry = old_map[i];
		while (entry != NULL) {
			if (put(hashmap, entry->key, entry->value) == 12)
				return 12;

			entry = entry->next;
		}
	}

	free_map(old_map, hashmap->capacity / 2);

	return 0;
}

int put(HashMap *hashmap, char *key, char *value)
{
	HashMapEntry *entry;
	int index, r;

	entry = get_entry(hashmap, key);
	if (entry != NULL) {
		free(entry->value);
		entry->value = str_dup(value);
		if (!entry->value)
			return 12;

		return 0;
	}

	// the load factor is 1
	hashmap->size += 1;
	if (hashmap->capacity <= hashmap->size) {
		r = rehash(hashmap);
		if (r == 12)
			return 12;
	}

	index = get_index(hashmap, key);
	entry = hashmap->map[index];
	if (!entry) {
		hashmap->map[index] = malloc(sizeof(HashMapEntry));
		if (!hashmap->map[index])
			return 12;

		hashmap->map[index]->next = NULL;
		hashmap->map[index]->key = str_dup(key);
		if (!hashmap->map[index]->key)
			return 12;

		hashmap->map[index]->value = str_dup(value);
		if (!hashmap->map[index]->value)
			return 12;

		return 0;
	}

	while (entry->next != NULL)
		entry = entry->next;

	entry->next = malloc(sizeof(HashMapEntry));
	if (!entry->next)
		return 12;

	entry->next->next = NULL;
	entry->next->key = str_dup(key);
	if (!entry->next->key)
		return 12;

	entry->next->value = str_dup(value);
	if (!entry->next->value)
		return 12;

	return 0;
}

char *get(HashMap *hashmap, char *key)
{
	HashMapEntry *entry;

	entry = get_entry(hashmap, key);
	if (entry == NULL)
		return NULL;

	return entry->value;
}

void delete(HashMap *hashmap, char *key)
{
	HashMapEntry *prev, *temp;
	int index;

	index = get_index(hashmap, key);
	if (!hashmap->map[index])
		return;

	if (!strcmp(hashmap->map[index]->key, key)) {
		temp = hashmap->map[index];
		hashmap->map[index] = hashmap->map[index]->next;
		temp->next = NULL;
		free_hashmap_entry(temp);
		return;
	}

	prev = hashmap->map[index];
	while (prev->next != NULL && strcmp(prev->next->key, key) != 0)
		prev = prev->next;

	// there was no such entry
	if (prev->next == NULL)
		return;

	temp = prev->next;
	prev->next = prev->next->next;
	temp->next = NULL;
	free_hashmap_entry(temp);
}

static void print_hashmap_entry(HashMapEntry *entry)
{
	if (entry->next != NULL)
		print_hashmap_entry(entry->next);

	printf("(%s, %s) ", entry->key, entry->value);
}

void print_hashmap(HashMap *hashmap)
{
	int i;

	printf("{ ");
	for (i = 0; i < hashmap->capacity; i++) {
		if (!hashmap->map[i])
			continue;

		print_hashmap_entry(hashmap->map[i]);
	}

	printf("}\n");
}
