//
// Created by alex on 13.03.2022.
//

#ifndef INC_1_MULTI_TEMA1_H
#define INC_1_MULTI_TEMA1_H

#define LINE_SIZE 256
#include <stdio.h>

typedef struct io_struct {
	char *outfile;
	char *infile;
} IOStruct;

int process_code(Vector *vector, HashMap *hashmap, IOStruct *ioStruct,
				 FILE *in, FILE *out,
				 int proceed_with_line,
				 int is_inside_if_block);

#endif //INC_1_MULTI_TEMA1_H
