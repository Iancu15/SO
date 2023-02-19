#ifndef LIN_UTILS_H_
#define LIN_UTILS_H_ 1

#include <stdlib.h>
#include <windows.h>

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

// modul normal de scriere la pozitia curenta in fisier
#define NORMAL_MODE 0

// modul de scriere in mod-ul append
// fiecare scriere e precedata de un seek la sfarsitul fisierului
#define APPEND_MODE 1

typedef struct _so_file {
	HANDLE fd;
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

	// modul de scriere(NORMAL_MODE si APPEND_MODE)
	int write_mode;

	// pi-ul copilului la popen, hProcess-ul e INVALID_HANDLE_VALUE la fopen
	PROCESS_INFORMATION pi;
} SO_FILE;

void set_error(SO_FILE *stream);

// implementare proprie strdup
char *str_dup(char *str);

// intoarce pointer la memoria alocata unde se afla cele 2 string-uri
// concatenate sau null in caz de eroare la alocarea de memorie
// cele 2 int-uri specifica daca parametrii sa fie dealocati sau nu
// 0 -> sa fie dealocat
// else -> sa nu fie dealocat
char *str_cat(char *str1, int to_be_freed1, char *str2, int to_be_freed2);

#endif
