//
// Created by alex on 15.03.2022.
//

#ifndef INC_1_MULTI_UTILS_H
#define INC_1_MULTI_UTILS_H

/**
 * own implementation of strdup
 */
char *str_dup(char *str);

/**
 * returns an allocated memory where the concatenated two strings reside
 * or null in case of an error in memory allocation
 * the two ints specify if the parameters are to be freed or not
 * 0 -> not to be freed
 * else -> to be freed
 */
char *str_cat(char *str1, int to_be_freed1, char *str2, int to_be_freed2);

#endif //INC_1_MULTI_UTILS_H
