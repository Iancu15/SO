/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include <fcntl.h>	/* O_RDWR, O_CREAT, O_TRUNC, O_WRONLY */
#include <unistd.h>	/* close */
#include <sys/mman.h>

#include "exec_parser.h"

static so_exec_t *exec;
static int exec_file;
static struct sigaction old_sa;

/* folosit pentru a citi din executabil */
static int so_xread(int fd, char* buffer, int size)
{
	ssize_t r, bytesWritten;

	bytesWritten = read(fd, buffer, size);
	if (bytesWritten < 0)
		return bytesWritten;

	/* read nu are garantia ca citeste toti octetii */
	while (bytesWritten < size) {
		r = read(fd, buffer + bytesWritten, size - bytesWritten);
		if (r < 0)
			return r;

		bytesWritten += r;
	}

	return 0;
}

static void loader_handler(int sig, siginfo_t *info, void *ucontext)
{
	unsigned int page_number_in_segment;
	size_t page_size;
	so_seg_t segment;
	int *is_page_mapped;
	uintptr_t address, segment_address;
	uintptr_t offset_inside_segment, page_address;
	void *mmap_return;
	unsigned int bytes_until_eof, bytes_read;
	char *buffer;
	int r, i;
	off_t offset;

	if (sig != SIGSEGV) {
		old_sa.sa_sigaction(sig, info, ucontext);
		return;
	}

	address = (uintptr_t) info->si_addr;
	page_size = getpagesize();
	for (i = 0; i < exec->segments_no; i++) {
		segment = exec->segments[i];
		segment_address = segment.vaddr;
		if (segment_address > address)
			continue;

		offset_inside_segment = address - segment_address;
		if (offset_inside_segment >= segment.mem_size)
			continue;

		page_number_in_segment = offset_inside_segment / page_size;
		is_page_mapped = segment.data;
		if (is_page_mapped[page_number_in_segment]) {
			old_sa.sa_sigaction(sig, info, ucontext);
			return;
		}

		/* mapez pagina cu PROT_WRITE pentru a scrie datele */
		/* din executabil in ea */
		/* MAP_FIXED trebuie sa fie cuplat cu MAP_PRIVATE sau */
		/* MAP_SHARED, am ales MAP_PRIVATE */
		/* MAP_ANONYMOUS pentru ca nu folosesc handler-ul */
		page_address = segment_address + page_number_in_segment * page_size;
		mmap_return = mmap((void *) page_address, page_size, PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (mmap_return == MAP_FAILED) {
			old_sa.sa_sigaction(sig, info, ucontext);
			return;
		}

		if (offset_inside_segment < segment.file_size) {
			buffer = malloc(sizeof(char) * segment.file_size);

			/* daca e ultima pagina din file_size voi citii maxim */
			/* page_size din executabil */
			/* restul de pagini de pana in file_size sunt pline */
			bytes_until_eof = segment.file_size - page_number_in_segment * page_size;
			bytes_read = page_size;
			if (bytes_until_eof < page_size)
				bytes_read = bytes_until_eof;

			/* mut cursor-ul la offset-ul din fisier al paginii */
			offset = segment.offset + page_number_in_segment * page_size;
			r = lseek(exec_file, offset, SEEK_SET);
			if (r != offset) {
				old_sa.sa_sigaction(sig, info, ucontext);
				return;
			}

			/* citesc datele din executabil */
			r = so_xread(exec_file, buffer, bytes_read);
			if (r < 0) {
				old_sa.sa_sigaction(sig, info, ucontext);
				return;
			}

			/* copiez in pagina datele din executabil */
			memcpy((void *) page_address, buffer, bytes_read);
			free(buffer);
		}

		/* actualizez permisiunile paginii la cele adevarate */
		r = mprotect((void *) page_address, page_size, segment.perm);
		if (r < 0) {
			old_sa.sa_sigaction(sig, info, ucontext);
			return;
		}

		is_page_mapped[page_number_in_segment] = 1;
		return;
	}

	old_sa.sa_sigaction(sig, info, ucontext);
}

int so_init_loader(void)
{
	struct sigaction sa;
	int r;

	/* setez rutina pentru SIGSEGV */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = loader_handler;
	r = sigaction(SIGSEGV, &sa, &old_sa);
	if (r < 0)
		return -1;

	return 0;
}

int so_execute(char *path, char *argv[])
{
	int r, i;
	unsigned int number_of_pages, page_size;
	so_seg_t *segment;

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	/* aloc campul data pentru fiecare segment */
	page_size = getpagesize();
	for (i = 0; i < exec->segments_no; i++) {
		segment = &(exec->segments[i]);
		number_of_pages = segment->mem_size / page_size + 1;
		segment->data = malloc(number_of_pages * sizeof(int));
		memset(segment->data, 0, number_of_pages * sizeof(int));
	}

	exec_file = open(path, O_RDONLY);
	if (exec_file < 0)
		return -1;

	so_start_exec(exec, argv);

	r = close(exec_file);
	if (r < 0)
		return -1;

	return 0;
}
