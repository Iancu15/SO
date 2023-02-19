//
// Created by alex on 13.03.2022.
//

#ifndef INC_1_MULTI_VECTOR_H
#define INC_1_MULTI_VECTOR_H

typedef struct vector {
	char **array;
	int capacity;
	int size;
} Vector;

/**
 * returns an empty vector of specified capacity,
 * null if there was an error
 */
Vector *empty_vector(int capacity);

/**
 * frees the vector
 */
void free_vector(Vector *vector);

/**
 * add the string literal to the vector,
 * returns 0 if it was allocated correctly, 12 otherwise
 */
int add(Vector *vector, char *elem);

/**
 * get the string at position pos, returns null if there
 * is no such pos
 */
char *string_at(Vector *vector, int pos);

/**
 * prints the contents of the vector
 */
void print_vector(Vector *vector);

#endif //INC_1_MULTI_VECTOR_H
