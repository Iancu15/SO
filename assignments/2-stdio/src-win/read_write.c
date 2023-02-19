//
// Created by alex on 25.03.2022.
//

#include "read_write.h"

int so_fgetc(SO_FILE *stream)
{
	int r, bytesRead;
	unsigned char c;

	if (stream->last_op == WRITE_OP) {
		set_error(stream);
		return SO_EOF;
	}

	if (stream->size == 0) {
		r = ReadFile(
			stream->fd,
			stream->buffer,
			BUF_SIZE,
			&bytesRead,
			NULL
		);

		if (r == FALSE && GetLastError() == ERROR_BROKEN_PIPE) {
			stream->eof = EOF_SET;
			set_error(stream);
			return SO_EOF;
		}

		// daca ultimul buffer care a fost golit "avea" eof,
		// atunci am ajuns la eof
		// daca am citit mai putin decat dimensiunea buffer-ului,
		// atunci sigur eof este in buffer
		if (stream->eof == EOF_IN_BUFFER)
			stream->eof = EOF_SET;
		else if (bytesRead < BUF_SIZE && bytesRead >= 0)
			stream->eof = EOF_IN_BUFFER;
		else
			stream->eof = NO_EOF;

		if (bytesRead == 0 || r == 0) {
			set_error(stream);
			return SO_EOF;
		}

		stream->size = bytesRead;
		stream->curr_buffer_index = 0;
	}

	stream->size--;
	stream->last_op = READ_OP;
	stream->offset++;

	c = stream->buffer[stream->curr_buffer_index++];
	return (int) c;
}

int so_fputc(int c, SO_FILE *stream)
{
	int r;

	if (stream->last_op == READ_OP) {
		set_error(stream);
		return SO_EOF;
	}

	stream->buffer[stream->curr_buffer_index++] = (unsigned char) c;
	stream->size++;
	stream->last_op = WRITE_OP;
	stream->offset++;

	// daca s-a umplut buffer-ul scriu in fd cu fflush
	if (stream->size == BUF_SIZE) {
		r = so_fflush(stream);
		if (r < 0) {
			set_error(stream);
			return SO_EOF;
		}
	}

	return c;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int full_size, i;
	int c;
	char *p = ptr;

	full_size = size * nmemb;

	// citesc caracter cu caracter din stream si pun in ptr
	for (i = 0; i < full_size; i++) {
		c = so_fgetc(stream);
		if (c == SO_EOF) {
			set_error(stream);
			return i;
		}

		*p = c;
		p++;
	}

	return nmemb;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int full_size, i;
	int r;
	unsigned char c;
	const char *p = ptr;

	full_size = size * nmemb;

	// pun caracter cu caracter in stream
	for (i = 0; i < full_size; i++) {
		c = p[i];
		r = so_fputc((int) c, stream);
		if (r == SO_EOF) {
			set_error(stream);
			return 0;
		}
	}

	return nmemb;
}
