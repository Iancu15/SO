//
// Created by alex on 25.03.2022.
//

#include "cursor_position.h"

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int r;

	if (!stream)
		return SO_EOF;

	if (stream->last_op == WRITE_OP)
		so_fflush(stream);

	// invalidez buffer-ul
	if (stream->last_op == READ_OP) {
		stream->size = 0;
		stream->curr_buffer_index = 0;
	}

	r = SetFilePointer(
			stream->fd,
			offset,
			NULL,
			whence
	);

	if (r == INVALID_SET_FILE_POINTER) {
		set_error(stream);
		return SO_EOF;
	}

	stream->offset = r;
	stream->last_op = NO_OP;
	stream->eof = NO_EOF;
	if (offset == 0L && whence == SEEK_END)
		stream->eof = EOF_SET;

	return 0;
}

long so_ftell(SO_FILE *stream)
{
	if (!stream)
		return -1L;

	return stream->offset;
}
