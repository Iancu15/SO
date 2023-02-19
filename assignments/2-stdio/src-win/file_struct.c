//
// Created by alex on 25.03.2022.
//

#include "file_struct.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *so_file;
	int creation_disposition;
	int err;
	int write_mode;

	write_mode = NORMAL_MODE;
	if (!strcmp(mode, "r") || !strcmp(mode, "r+")) {
		creation_disposition = OPEN_EXISTING;
	} else if (!strcmp(mode, "w") || !strcmp(mode, "w+")) {
		creation_disposition = TRUNCATE_EXISTING;
	} else if (!strcmp(mode, "a") || !strcmp(mode, "a+")) {
		creation_disposition = OPEN_EXISTING;
		write_mode = APPEND_MODE;
	} else
		return NULL;

	so_file = malloc(sizeof(SO_FILE));
	if (!so_file)
		return NULL;

	so_file->fd = CreateFile(
		pathname,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (so_file->fd == INVALID_HANDLE_VALUE) {
		err = 1;
		// daca pentru w, w+, a sau a+ a dat fail incercand sa
		// deschida un fisier existent, atunci trebuie sa il creeze
		if (strcmp(mode, "r") && strcmp(mode, "r+")) {
			so_file->fd = CreateFile(
				pathname,
				GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

			if (so_file->fd != INVALID_HANDLE_VALUE)
				err = 0;
		}

		if (err) {
			free(so_file);
			return NULL;
		}
	}

	so_file->curr_buffer_index = 0;
	so_file->size = 0;
	so_file->offset = 0;
	so_file->error = 0;
	so_file->eof = NO_EOF;
	so_file->last_op = NO_OP;
	so_file->pi.hProcess = INVALID_HANDLE_VALUE;
	so_file->write_mode = write_mode;

	return so_file;
}

int so_fclose(SO_FILE *stream)
{
	int r;

	if (!stream)
		return SO_EOF;

	if (stream->last_op == WRITE_OP) {
		r = so_fflush(stream);
		if (r < 0) {
			free(stream);
			return SO_EOF;
		}
	}

	r = CloseHandle(stream->fd);
	if (r == 0) {
		free(stream);
		return SO_EOF;
	}

	free(stream);

	return 0;
}
