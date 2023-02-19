/**
 * SO
 * Lab #6, Virtual Memory
 *
 * Task #4, Linux
 *
 * Implementing 'cp' with mmap
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

int main(int argc, char *argv[])
{
	int fdin;
	int fdout;
	int rc;
    int r;
    int size;
	char *src;
	char *dst;
	struct stat statbuf;

	DIE(argc != 3, "Usage: ./mycp <from_file> <to_file>");

	/* Open input and output files */
	fdin = open(argv[1], O_RDONLY);
	DIE(fdin == -1, "open fdin");

	/* Open/create the output file */
	fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
	DIE(fdout == -1, "open fdout");

	/* TODO - Truncate the output file to the input file size */
    r = fstat(fdin, &statbuf);
    DIE(r == -1, "fstat");
    size = statbuf.st_size;
    r = ftruncate(fdout, size);
    DIE(r == -1, "ftruncate");

	/* TODO - mmap the input and output file */
    src = mmap(NULL, size, PROT_READ, MAP_SHARED, fdin, 0);
    DIE(src == MAP_FAILED, "mmap src");

    dst = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0);
    DIE(dst == MAP_FAILED, "mmap dst");

	/* TODO - Copy the input file to the output file */
    memcpy(dst, src, size);
    msync(dst, size, MS_SYNC);

	/* TODO - Clean up */
	rc = munmap(src, size);
	DIE(rc == -1, "munmap src");

	rc = munmap(dst, size);
	DIE(rc == -1, "munmap dst");

	/* Close files */
	rc = close(fdin);
	DIE(rc == -1, "close source");

	rc = close(fdout);
	DIE(rc == -1, "close destination");

	return 0;
}
