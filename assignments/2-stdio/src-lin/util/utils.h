#ifndef LIN_UTILS_H_
#define LIN_UTILS_H_ 1

#include <stdlib.h>

#define BUF_SIZE 4096

// ultima operatia nu a fost una de scriere sau de citire
#define NO_OP 0

// ultima operatie a fost una de scriere
#define WRITE_OP 1

// ultima operatie a fost una de citire
#define READ_OP 2

// nu am ajuns la eof
#define NO_EOF 0

// dupa ce se citesc elementele din buffer se ajunge la eof
#define EOF_IN_BUFFER 1

// s-a ajuns la eof
#define EOF_SET 2

typedef struct _so_file {
	int fd;
	char buffer[BUF_SIZE];
	int eof;
	int last_op;

	// pozitia curenta de scriere sau citire din buffer
	int curr_buffer_index;

	// dimensiunea buffer-ului
	int size;

	// offset-ul de la inceputul fisierul
	int offset;

	// flag-ul de error(0 nesetat, 1 setat)
	int error;

	// pid-ul copilului la popen, e -1 la fopen
	int pid;
} SO_FILE;

void set_error(SO_FILE *stream);

#endif
