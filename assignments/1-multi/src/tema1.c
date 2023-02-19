//
// Created by alex on 11.03.2022.
//

#include "hashmap.h"
#include "vector.h"
#include "tema1.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

IOStruct *empty_io_struct()
{
	IOStruct *ioStruct;

	ioStruct = malloc(sizeof(IOStruct));
	if (!ioStruct)
		return NULL;

	ioStruct->infile = NULL;
	ioStruct->outfile = NULL;
	return ioStruct;
}

void free_io_struct(IOStruct *ioStruct)
{
	if (ioStruct->infile != NULL)
		free(ioStruct->infile);

	if (ioStruct->outfile != NULL)
		free(ioStruct->outfile);

	free(ioStruct);
}

void print_io_struct(IOStruct *ioStruct)
{
	printf("/ in:");
	if (ioStruct->infile == NULL)
		printf("null");
	else
		printf("%s", ioStruct->infile);

	printf(" out:");
	if (ioStruct->outfile == NULL)
		printf("null");
	else
		printf("%s", ioStruct->outfile);

	printf(" /\n");
}

int process_args_mapping(HashMap *hashmap, char *str)
{
	char *token, *key;
	int r;

	token = strtok(str, "=");
	key = str_dup(token);
	if (!key)
		return 12;

	token = strtok(NULL, "=");
	if (token != NULL)
		r = put(hashmap, key, token);
	else
		r = put(hashmap, key, "");

	free(key);
	return r;
}

int process_args(HashMap *hashmap, Vector *vector, IOStruct *ioStruct,
				 int argc, char *argv[])
{
	int i, r, cond;
	char *dir, *s;
	HashMap *h;

	for (i = 1; i < argc; i++) {
		if (!strcmp("-D", argv[i])) {
			if (i == argc - 1)
				exit(-1);

			if (!process_args_mapping(hashmap, argv[++i]))
				continue;

			return 12;
		}

		if (!strcmp("-I", argv[i])) {
			if (i == argc - 1)
				exit(-1);

			dir = str_cat(argv[++i], 0, "/", 0);
			if (!dir)
				return 12;

			if (!add(vector, dir)) {
				free(dir);
				continue;
			}

			return 12;
		}

		if (!strcmp("-o", argv[i])) {
			if (i == argc - 1 || ioStruct->outfile != NULL)
				exit(-1);

			ioStruct->outfile = str_dup(argv[++i]);
			if (ioStruct->outfile != NULL)
				continue;

			return 12;
		}

		if (strlen(argv[i]) > 2 && argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'D':
				h = hashmap;
				s = &argv[i][2];
				r = process_args_mapping(h, s);
				if (r == 12)
					return 12;

				continue;

			case 'I':
				dir = str_cat(&argv[i][2], 0, "/", 0);
				if (!dir)
					return 12;

				if (add(vector, dir) == 12) {
					free(dir);
					return 12;
				}

				continue;

			case 'o':
				if (ioStruct->outfile != NULL)
					exit(-1);

				s = &argv[i][2];
				ioStruct->outfile = str_dup(s);
				if (!ioStruct->outfile)
					return 12;

				continue;
			}
		}

		cond = !ioStruct->outfile && ioStruct->infile != NULL;
		if (i == argc - 1 && cond) {
			ioStruct->outfile = str_dup(argv[i]);
			if (!ioStruct->outfile)
				return 12;

			continue;
		}

		if (ioStruct->infile != NULL)
			exit(-1);

		ioStruct->infile = str_dup(argv[i]);
		if (!ioStruct->infile)
			return 12;
	}

	return 0;
}

char *process_define_mapping(char buffer[])
{
	char *value, *token;
	int len;

	value = str_dup("");
	if (!value)
		return NULL;

	token = strtok(buffer, " \t\n");
	while (token != NULL) {
		// in case \ is a standalone token like in "1 \"
		// I ignore it
		if (!strcmp(token, "\\")) {
			token = strtok(NULL, " \t\n");
			continue;
		}

		value = str_cat(value, 1, token, 0);
		if (!value)
			return NULL;

		// in case it's concatenated to another token like in "2\"
		// I remove it
		token = strtok(NULL, " \t\n");
		len = strlen(value);
		if (token == NULL && value[len - 1] == '\\')
			value[len - 1] = '\0';

		value = str_cat(value, 1, " ", 0);
		if (!value)
			return NULL;
	}

	return value;
}

int process_define(HashMap *hashmap, FILE *in, int is_multi_line)
{
	char buffer[LINE_SIZE];
	char *key, *value, *token, *mapping;
	int another_line, len;

	another_line = is_multi_line;
	token = strtok(NULL, " ");
	key = str_dup(token);
	if (!key)
		return 12;

	value = process_define_mapping(NULL);
	if (!value)
		return 12;

	while (another_line) {
		fgets(buffer, LINE_SIZE, in);
		another_line = buffer[strlen(buffer) - 2] == '\\';
		mapping = process_define_mapping(buffer);
		if (!mapping)
			return 12;

		value = str_cat(value, 1, mapping, 1);
		if (!value)
			return 12;
	}

	// the bonus ending space is removed
	len = strlen(value);
	if (len > 0)
		value[len - 1] = '\0';

	if (put(hashmap, key, value) == 12)
		return 12;

	free(key);
	free(value);
	return 0;
}

char *replace_define_str(HashMap *hashmap, char buffer[])
{
	char delim[30];
	char new_line[LINE_SIZE];
	char token[LINE_SIZE];
	char *ptr, *value, *delim_ptr;
	int new_line_i, token_i, cond;

	new_line_i = 0;
	token_i = 0;
	strcpy(delim, "\t []{}<>=+-*/%!&|^.,:;()\\\n");
	for (ptr = buffer; *ptr != '\0'; ptr++) {
		delim_ptr = strchr(delim, *ptr);
		if (!delim_ptr)
			token[token_i++] = *ptr;

		cond = delim_ptr != NULL || *(ptr + 1) == '\0';
		if (token_i != 0 && cond) {
			token[token_i] = '\0';
			new_line[new_line_i] = '\0';
			value = get(hashmap, token);
			if (value != NULL) {
				value = replace_define_str(hashmap, value);
				if (!value)
					return NULL;

				strcat(new_line, value);
				new_line_i += strlen(value);
				free(value);
			} else {
				strcat(new_line, token);
				new_line_i += token_i;
			}

			token_i = 0;
		}

		if (delim_ptr != NULL)
			new_line[new_line_i++] = *ptr;
	}

	new_line[new_line_i] = '\0';
	return str_dup(new_line);
}

void process_undef(HashMap *hashmap)
{
	char *token;

	token = strtok(NULL, "\n ");
	delete(hashmap, token);
}

/**
 * if the token is an integer different from 0 it return 1
 * else it returns 0
 */
int test_cond(char *token)
{
	int len, i;

	if (token[0] == '0')
		return 0;

	len = strlen(token);
	for (i = 0; i < len; i++) {
		if (!strchr("0123456789", token[i]))
			return 0;
	}

	return 1;
}

int process_if(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
			   FILE *in, FILE *out)
{
	char *token, *value;

	token = strtok(NULL, "\n ");
	value = get(hashmap, token);
	if (value == NULL)
		value = token;

	if (test_cond(value))
		return process_code(vector, hashmap, ioStruct, in, out, 1, 1);

	return 13;
}

int process_else(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
				 FILE *in, FILE *out)
{
	return process_code(vector, hashmap, ioStruct, in, out, 1, 1);
}

int process_if_def(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
				   FILE *in, FILE *out, int type)
{
	char *token, *value;

	token = strtok(NULL, "\n ");
	value = get(hashmap, token);
	if ((type == 1 && value != NULL) || (!type && !value))
		return process_code(vector, hashmap, ioStruct, in, out, 1, 1);

	return 13;
}

int process_header(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
				   FILE *out, char *path)
{
	FILE *in;
	int r;

	in = fopen(path, "r");
	if (!in)
		return 13;

	r = process_code(vector, hashmap, ioStruct, in, out, 1, 0);
	fclose(in);
	return r;
}

int process_include(Vector *vector, HashMap *hashmap,
					IOStruct *ioStruct, FILE *out)
{
	char *token1, *token2, *path, *header, *infile_dir;
	int i, r;

	header = strtok(NULL, "\n\" ");
	if (!ioStruct->infile) {
		r = process_header(vector, hashmap, ioStruct, out, header);
		if (r != 13)
			return r;
	} else {
		token1 = strtok(ioStruct->infile, "/");
		token2 = strtok(NULL, "/");
		infile_dir = strdup("");
		if (!infile_dir)
			return 12;

		while (token2 != NULL) {
			infile_dir = str_cat(infile_dir, 1, token1, 0);
			if (!infile_dir)
				return 12;

			infile_dir = str_cat(infile_dir, 1, "/", 0);
			if (!infile_dir)
				return 12;

			token1 = token2;
			token2 = strtok(NULL, "/");
		}

		path = str_cat(infile_dir, 1, header, 0);
		if (!path)
			return 12;

		r = process_header(vector, hashmap, ioStruct, out, path);
		free(path);
		if (r != 13)
			return r;
	}

	for (i = 0; i < vector->size; i++) {
		path = str_cat(vector->array[i], 0, header, 0);
		if (!path)
			return 12;

		r = process_header(vector, hashmap, ioStruct, out, path);
		free(path);
		if (r != 13)
			return r;
	}

	exit(-1);
}

int process_code(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
				 FILE *in, FILE *out,
				 int proceed_with_line,
				 int is_inside_if_block)
{
	char buffer[LINE_SIZE];
	char *token, *processed_line;
	int is_multi_line, r;
	HashMap *h;
	Vector *v;
	IOStruct *io;

	v = vector;
	h = hashmap;
	io = ioStruct;
	while (fgets(buffer, LINE_SIZE, in) != NULL) {
		if (buffer[0] == '\n')
			continue;

		if (buffer[0] == '#') {
			is_multi_line = buffer[strlen(buffer) - 2] == '\\';
			token = strtok(buffer, "\n ");
			if (!strcmp(token, "#define") && proceed_with_line) {
				r = process_define(hashmap, in, is_multi_line);
				if (r == 12)
					return 12;

				continue;
			}

			if (!strcmp(token, "#undef") && proceed_with_line) {
				process_undef(hashmap);

				continue;
			}

			if (!strcmp(token, "#if")) {
				r = process_if(vector, h, ioStruct, in, out);
				if (r == 12)
					return 12;
				else if (r == 13)
					proceed_with_line = 0;

				continue;
			}

			if (!strcmp(token, "#elif")) {
				if (!proceed_with_line) {
					r = process_if(v, h, io, in, out);
					if (r == 12)
						return 12;
					else if (r == 13)
						proceed_with_line = 0;
					else
						proceed_with_line = 1;
				} else
					proceed_with_line = 0;

				continue;
			}

			if (!strcmp(token, "#else")) {
				if (!proceed_with_line) {
					r = process_else(v, h, io, in, out);
					if (r == 12)
						return 12;

					proceed_with_line = 1;
				} else
					proceed_with_line = 0;

				continue;
			}

			if (!strcmp(token, "#endif")) {
				proceed_with_line = 1;
				if (is_inside_if_block)
					return 0;
			}

			if (!strcmp(token, "#ifdef")) {
				r = process_if_def(v, h, io, in, out, 1);
				if (r == 12)
					return 12;
				else if (r == 13)
					proceed_with_line = 0;
			}

			if (!strcmp(token, "#ifndef")) {
				r = process_if_def(v, h, io, in, out, 0);
				if (r == 12)
					return 12;
				else if (r == 13)
					proceed_with_line = 0;
			}

			if (!strcmp(token, "#include") && proceed_with_line) {
				r = process_include(v, h, io, out);
				if (r == 12)
					return 12;
			}
		} else if (proceed_with_line) {
			processed_line = replace_define_str(hashmap, buffer);
			if (!processed_line)
				return 12;

			fprintf(out, "%s", processed_line);
			free(processed_line);
		}
	}

	return 0;
}

int process_infile(Vector *vector, HashMap *hashmap, IOStruct *ioStruct)
{
	FILE *in, *out;

	in = stdin;
	if (ioStruct->infile != NULL) {
		in = fopen(ioStruct->infile, "r");
		if (!in)
			exit(-1);
	}

	out = stdout;
	if (ioStruct->outfile != NULL)
		out = fopen(ioStruct->outfile, "w");

	if (process_code(vector, hashmap, ioStruct, in, out, 1, 0) == 12)
		return 12;

	if (ioStruct->infile != NULL)
		fclose(in);

	if (ioStruct->outfile != NULL)
		fclose(out);

	return 0;
}

int main(int argc, char *argv[])
{
	Vector *vector;
	HashMap *hashmap;
	IOStruct *ioStruct;

	ioStruct = empty_io_struct();
	if (!ioStruct)
		return 12;

	vector = empty_vector(100);
	if (!vector)
		return 12;

	hashmap = empty_hashmap(1000);
	if (!hashmap)
		return 12;

	if (process_args(hashmap, vector, ioStruct, argc, argv) == 12)
		return 12;

	if (process_infile(vector, hashmap, ioStruct) == 12)
		return 12;

	free_io_struct(ioStruct);
	free_hashmap(hashmap);
	free_vector(vector);

	return 0;
}
