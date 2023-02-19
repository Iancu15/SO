//
// Created by alex on 25.03.2022.
//

#include "buffering.h"

int so_fflush(SO_FILE *stream)
{
	ssize_t r, bytesWritten;

	if (!stream)
		return SO_EOF;

	bytesWritten = write(stream->fd, stream->buffer, stream->size);
	if (bytesWritten < 0) {
		set_error(stream);
		return SO_EOF;
	}

	// write nu are garantia ca scrie toti octetii
	while (bytesWritten < stream->size) {
		r = write(stream->fd, stream->buffer + bytesWritten, stream->size - bytesWritten);
		if (r < 0) {
			set_error(stream);
			return SO_EOF;
		}

		bytesWritten += r;
	}

	stream->size = 0;
	stream->curr_buffer_index = 0;
	stream->last_op = NO_OP;

	return 0;
}
