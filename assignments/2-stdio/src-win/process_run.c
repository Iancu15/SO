#include "process_run.h"
#include <stdio.h>

SO_FILE *so_popen(const char *command, const char *type)
{
	int r;
	HANDLE fd_read;
	HANDLE fd_write;
	char *cmd_line;
	SO_FILE *so_file;
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;

	if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0)
		return NULL;

	cmd_line = str_dup("cmd /C ");
	if (!cmd_line)
		return NULL;

	cmd_line = str_cat(cmd_line, 1, (char *) command, 0);
	if (!cmd_line)
		return NULL;

	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	r = CreatePipe(
		&fd_read,
		&fd_write,
		&sa,
		0
	);

	if (r == 0)
		return NULL;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!strcmp(type, "r")) {
		r = SetHandleInformation(fd_read, HANDLE_FLAG_INHERIT, 0);
		if (r == 0)
			return NULL;

		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = fd_write;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	} else {
		r = SetHandleInformation(fd_write, HANDLE_FLAG_INHERIT, 0);
		if (r == 0)
			return NULL;

		si.hStdInput = fd_read;
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	}

	si.dwFlags |= STARTF_USESTDHANDLES;
	r = CreateProcess(
		NULL,
		cmd_line,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	free(cmd_line);
	//daca a avut loc o eroare inchid capetele
	if (r == 0) {
		r = CloseHandle(fd_read);
		if (r == 0)
			return NULL;

		r = CloseHandle(fd_write);
		if (r == 0)
			return NULL;
	} else {
		so_file = malloc(sizeof(SO_FILE));
		if (!so_file)
			return NULL;

		/* daca e in modul de citire, atunci fd-ul sau va fi capatul */
		/* de citire al pipe-ului, analog pentru scriere */
		so_file->curr_buffer_index = 0;
		so_file->size = 0;
		so_file->offset = 0;
		so_file->error = 0;
		so_file->eof = 0;
		so_file->last_op = NO_OP;
		so_file->pi = pi;
		so_file->write_mode = NORMAL_MODE;
		if (!strcmp(type, "r")) {
			r = CloseHandle(fd_write);
			if (r == 0)
				return NULL;

			so_file->fd = fd_read;
		} else {
			so_file->fd = fd_write;
		}

		return so_file;
	}

	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	int status, r;
	HANDLE pid;

	if (!stream)
		return SO_EOF;

	if (stream->pi.hProcess == INVALID_HANDLE_VALUE) {
		set_error(stream);
		return SO_EOF;
	}

	/* pentru cazul in care copilul citeste trebuie inchis stream-ul */
	/* de dinainte pentru ca fd-ul de scriere al pipe-ului sa se */
	/* inchida si copilul sa fie astfel constient ca se poate termina */
	pid = stream->pi.hProcess;
	so_fclose(stream);
	r = WaitForSingleObject(pid, INFINITE);
	if (r == WAIT_FAILED)
		return SO_EOF;

	r = GetExitCodeProcess(pid, &status);
	if (r == 0)
		return SO_EOF;

	return status;
}
