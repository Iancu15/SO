//
// Created by alex on 25.03.2022.
//

#include "process_run.h"

void dealloc_args(char *args[])
{
	int arg_index;
	char *arg;

	arg_index = 0;
	arg = args[arg_index++];
	while (arg != NULL) {
		free(arg);
		arg = args[arg_index++];
	}
}

SO_FILE *so_popen(const char *command, const char *type)
{
	pid_t pid;
	int r;
	int fd[2];
	char *args[MAX_ARGS];
	SO_FILE *so_file;

	if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0)
		return NULL;

	args[0] = strdup("sh");
	if (!args[0])
		return NULL;

	args[1] = strdup("-c");
	if (!args[1])
		return NULL;

	args[2] = strdup(command);
	if (!args[2])
		return NULL;

	args[3] = NULL;
	r = pipe(fd);
	if (r < 0)
		return NULL;

	pid = fork();

	// daca a avut loc o eroare inchid capetele
	if (pid < 0) {
		r = close(fd[READ_PIPE]);
		if (r < 0)
			return NULL;

		r = close(fd[WRITE_PIPE]);
		if (r < 0)
			return NULL;

		dealloc_args(args);
	} else if (pid == 0) {
		// pentru modul citire, copilul va folosi pipe-ul de scriere si
		// voi redirecta stdout-ul lui la acel pipe, pentru modul de
		// scriere, copilul va folosi pipe-ul de citire si asa voi
		// redirecta stdin-ul la acel pipe
		if (!strcmp(type, "r")) {
			r = close(fd[READ_PIPE]);
			if (r < 0)
				return NULL;

			r = dup2(fd[WRITE_PIPE], STDOUT_FILENO);
			if (r < 0)
				return NULL;

			r = close(fd[WRITE_PIPE]);
			if (r < 0)
				return NULL;
		} else {
			r = close(fd[WRITE_PIPE]);
			if (r < 0)
				return NULL;

			r = dup2(fd[READ_PIPE], STDIN_FILENO);
			if (r < 0)
				return NULL;

			r = close(fd[READ_PIPE]);
			if (r < 0)
				return NULL;
		}

		execvp(args[0], (char * const *) args);
	} else {
		so_file = malloc(sizeof(SO_FILE));
		if (!so_file)
			return NULL;

		so_file->curr_buffer_index = 0;
		so_file->size = 0;
		so_file->offset = 0;
		so_file->error = 0;
		so_file->eof = 0;
		so_file->last_op = NO_OP;
		so_file->pid = pid;

		// daca e in modul de citire, atunci fd-ul sau va fi capatul
		// de citire al pipe-ului, analog pentru scriere
		if (!strcmp(type, "r")) {
			r = close(fd[WRITE_PIPE]);
			if (r < 0)
				return NULL;

			so_file->fd = fd[READ_PIPE];
		} else {
			r = close(fd[READ_PIPE]);
			if (r < 0)
				return NULL;

			so_file->fd = fd[WRITE_PIPE];
		}

		dealloc_args(args);
		return so_file;
	}

	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	int status, r, pid;

	if (!stream)
		return SO_EOF;

	if (stream->pid < 0) {
		set_error(stream);
		return SO_EOF;
	}

	pid = stream->pid;
	// pentru cazul in care copilul citeste trebuie inchis stream-ul
	// de dinainte pentru ca fd-ul de scriere al pipe-ului sa se
	// inchida si copilul sa fie astfel constient ca se poate termina
	so_fclose(stream);
	r = waitpid(pid, &status, 0);
	if (r < 0)
		return SO_EOF;

	return status;
}
