//
// Created by alex on 11.03.2022.
//

#ifndef INC_1_MULTI_HASHMAP_H
#define INC_1_MULTI_HASHMAP_H

typedef struct hashmap_entry {
	char *key;
	char *value;
	struct hashmap_entry *next;
} HashMapEntry;

typedef struct hashmap {
	HashMapEntry **map;
	int capacity;
	int size;
} HashMap;

/**
 * returns an empty hashmap
 * if there was an error with malloc it returns null
 */
HashMap *empty_hashmap(int capacity);

/**
 * frees the memory of the hashmap
 */
void free_hashmap(HashMap *hashmap);

/**
 * adds the key, value pair in the hashmap, if there was already
 * a mapping for the key then the old value is replaced,
 * it returns 0 if nothing went wrong
 * if realloc fails in case of increasing capacity then it returns 12
 */
int put(HashMap *hashmap, char *key, char *value);

/**
 * returns the value mapped to the key
 * if there is no such value it returns null
 */
char *get(HashMap *hashmap, char *key);

/**
 * deletes the entry associated with the key
 * if there is no such key in the map then it does nothing
 */
void delete(HashMap *hashmap, char *key);

/**
 * prints the hashmap entries
 */
void print_hashmap(HashMap *hashmap);

#endif //INC_1_MULTI_HASHMAP_H
