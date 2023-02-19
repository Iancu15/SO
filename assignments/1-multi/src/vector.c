//
// Created by alex on 13.03.2022.
//

#include "vector.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Vector *empty_vector(int capacity)
{
	Vector *vector;
	int i;

	vector = malloc(sizeof(Vector));
	if (!vector)
		return NULL;

	vector->size = 0;
	vector->capacity = capacity;
	vector->array = malloc(capacity * sizeof(char *));
	if (!vector->array)
		return NULL;

	for (i = 0; i < vector->capacity; i++)
		vector->array[i] = NULL;

	return vector;
}

void free_vector(Vector *vector)
{
	int i;

	for (i = 0; i < vector->size; i++)
		free(vector->array[i]);

	free(vector->array);
	free(vector);
}

int add(Vector *vector, char *elem)
{
	int new_capacity;

	vector->array[vector->size] = str_dup(elem);
	if (!vector->array[vector->size])
		return 12;

	vector->size++;
	if (vector->capacity <= vector->size) {
		vector->capacity *= 2;
		new_capacity = vector->capacity * sizeof(char *);
		vector->array = realloc(vector->array, new_capacity);
		if (!vector->array)
			return 12;
	}

	return 0;
}

char *string_at(Vector *vector, int pos)
{
	if (pos < 0 || pos >= vector->size)
		return NULL;

	return vector->array[pos];
}

void print_vector(Vector *vector)
{
	int i;

	printf("[ ");
	for (i = 0; i < vector->size; i++)
		printf("%s ", vector->array[i]);

	printf("]\n");
}
