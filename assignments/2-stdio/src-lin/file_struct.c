//
// Created by alex on 25.03.2022.
//

#include "file_struct.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *so_file;
	int flags;

	if (!strcmp(mode, "r"))
		flags = O_RDONLY;
	else if (!strcmp(mode, "r+"))
		flags = O_RDWR;
	else if (!strcmp(mode, "w"))
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "w+"))
		flags = O_RDWR | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "a"))
		flags = O_WRONLY | O_APPEND | O_CREAT;
	else if (!strcmp(mode, "a+"))
		flags = O_RDWR | O_APPEND | O_CREAT;
	else
		return NULL;

	so_file = malloc(sizeof(SO_FILE));
	if (!so_file)
		return NULL;

	so_file->fd = open(pathname, flags, PERMISSIONS_MODE);
	if (so_file->fd < 0) {
		free(so_file);
		return NULL;
	}

	so_file->curr_buffer_index = 0;
	so_file->size = 0;
	so_file->offset = 0;
	so_file->error = 0;
	so_file->eof = NO_EOF;
	so_file->last_op = NO_OP;
	so_file->pid = -1;

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

	r = close(stream->fd);
	if (r < 0) {
		free(stream);
		return SO_EOF;
	}

	free(stream);

	return 0;
}
