/**
 * SO
 * Lab #07
 *
 * Task #5, Linux
 *
 * Printing order
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	fprintf(stdout, "A");
	fprintf(stderr, "B");

	sleep(5);
	return 0;
}
