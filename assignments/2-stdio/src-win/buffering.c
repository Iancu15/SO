//
// Created by alex on 25.03.2022.
//

#include "buffering.h"

int so_fflush(SO_FILE *stream)
{
	int r, bytesWrittenTotal, bytesWritten;

	// daca sunt in modul append scriu la final
	if (stream->write_mode == APPEND_MODE) {
		r = SetFilePointer(
			stream->fd,
			0,
			NULL,
			FILE_END
		);

		if (r == INVALID_SET_FILE_POINTER) {
			set_error(stream);
			return SO_EOF;
		}

		stream->offset = r;
		stream->eof = EOF_SET;
	}

	if (!stream)
		return SO_EOF;

	r = WriteFile(
		stream->fd,
		stream->buffer,
		stream->size,
		&bytesWrittenTotal,
		NULL
	);

	if (r == 0) {
		set_error(stream);
		return SO_EOF;
	}

	// writefile nu are garantia ca scrie toti octetii
	while (bytesWrittenTotal < stream->size) {
		r = WriteFile(
			stream->fd,
			stream->buffer + bytesWrittenTotal,
			stream->size - bytesWrittenTotal,
			&bytesWritten,
			NULL
		);

		if (r == 0) {
			set_error(stream);
			return SO_EOF;
		}

		bytesWrittenTotal += bytesWritten;
	}

	stream->size = 0;
	stream->curr_buffer_index = 0;
	stream->last_op = NO_OP;

	return 0;
}
