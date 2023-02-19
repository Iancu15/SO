/**
 * SO
 * Lab #9
 *
 * Task #5, Windows
 *
 * Timer Queue
 */

/* do not use UNICODE */
#undef _UNICODE
#undef UNICODE

#define _WIN32_WINNT    0x500
#include <windows.h>
#include <stdio.h>

#include "utils.h"

#define PERIOD       1000
#define TIMES        3

HANDLE finished;

VOID CALLBACK TimerFunction(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	static int count;
	BOOL bRet;

	printf("'TimerFunction' has been called and count is %d\n", count);

	/* TODO - Check if we must increment counter or finish */
	if (count == TIMES - 1) {
		ReleaseSemaphore(finished, 1, NULL);
	} else {
		count++;
	}

}

int main(void)
{
	HANDLE timer_queue;
	HANDLE timer;
	BOOL bRet;
	DWORD dwRet;

	setbuf(stdout, NULL);

	/* TODO - Create a TimerQueue */
	timer_queue = CreateTimerQueue();

	/* TODO - Create a semaphore/event */
	finished = CreateSemaphore(NULL, 0, 1, NULL);

	/* TODO - Create a timer and associate it with the timer queue */
	CreateTimerQueueTimer(
		&timer,
		timer_queue,
		TimerFunction,
		NULL,
		0,
		PERIOD,
		0
	);

	/* TODO - Wait for the semaphore/event to be set so we can free resources */
	WaitForSingleObject(finished, INFINITE);

	/* TODO - Delete the timer queue and its timers */
	DeleteTimerQueue(
		timer_queue
	);

	return 0;
}
