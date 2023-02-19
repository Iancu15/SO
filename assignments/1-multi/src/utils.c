//
// Created by alex on 15.03.2022.
//

#include "utils.h"
#include <stdlib.h>
#include <string.h>

char *str_dup(char *str)
{
	char *duplicated_str;

	duplicated_str = malloc(sizeof(char) * (strlen(str) + 1));
	if (!duplicated_str)
		return NULL;

	strcpy(duplicated_str, str);
	return duplicated_str;
}

char *str_cat(char *str1, int to_be_freed1, char *str2, int to_be_freed2)
{
	char *concatenated_str;
	int concatenated_str_size;

	concatenated_str_size = strlen(str1) + strlen(str2) + 1;
	concatenated_str = malloc(sizeof(char) * concatenated_str_size);
	if (!concatenated_str)
		return NULL;

	strcpy(concatenated_str, str1);
	strcat(concatenated_str, str2);
	if (to_be_freed1)
		free(str1);

	if (to_be_freed2)
		free(str2);

	return concatenated_str;
}
