//
// Created by alex on 25.03.2022.
//

#include "cursor_position.h"

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	off_t r;

	if (!stream)
		return SO_EOF;

	if (stream->last_op == WRITE_OP)
		so_fflush(stream);

	// invalidez buffer-ul
	if (stream->last_op == READ_OP) {
		stream->size = 0;
		stream->curr_buffer_index = 0;
	}

	r = lseek(stream->fd, offset, whence);
	if (r < 0) {
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
