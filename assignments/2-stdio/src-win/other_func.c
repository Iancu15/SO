//
// Created by alex on 25.03.2022.
//

#include "other_func.h"

HANDLE so_fileno(SO_FILE *stream)
{
	if (!stream)
		return INVALID_HANDLE_VALUE;

	return stream->fd;
}

int so_feof(SO_FILE *stream)
{
	if (!stream)
		return 0;

	if (stream->eof == EOF_SET)
		return stream->eof;

	return 0;
}

int so_ferror(SO_FILE *stream)
{
	if (!stream)
		return 0;

	return stream->error;
}
