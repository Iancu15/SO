#include "util/so_scheduler.h"
#include <stdio.h>
#include <stdlib.h>

#define SO_TEST_FAIL 0
#define SO_TEST_SUCCESS 1
#define SO_MAX_UNITS 32
#define SO_VERBOSE_ERROR 1

#define SO_DEV0		0
#define SO_DEV1		1
#define SO_DEV2		2
#define SO_DEV3		3

#define SO_PREEMPT_UNITS	3

static unsigned int exec_time;
static unsigned int exec_devs;
static unsigned int last_priority;
static unsigned int exec_priority;
static unsigned int test_exec_status = SO_TEST_FAIL;

/* debugging macro */
#ifdef SO_VERBOSE_ERROR
#define so_error(msg, ...) fprintf(stderr, "ERR: " msg "\n", ##__VA_ARGS__)
#else
#define so_error(msg, ...)
#endif

/* shows the message and exits */
#define so_fail(msg)   \
	do                 \
	{                  \
		so_error(msg); \
		exit(-1);      \
	} while (0)

/* returns unsigned random value between _min and _max - 1 */
#define get_rand(min, max) ((rand() % (max - min)) + min)

static inline tid_t get_tid(void)
{
	return pthread_self();
}

static inline int equal_tids(tid_t t1, tid_t t2)
{
	return pthread_equal(t1, t2);
}

/* useful defines */
static inline int this_tid(tid_t t)
{
	return pthread_equal(t, get_tid());
}

#define SO_MAX_UNITS 32
#define get_rand(min, max) ((rand() % (max - min)) + min)

static unsigned int test_sched_fork_handler_runs;
static unsigned int test_sched_fork_runs;

static void test_sched_handler_10_worker(unsigned int prio)
{
	test_sched_fork_handler_runs++;
}

static void test_sched_handler_10_master(unsigned int prio)
{
	unsigned int run;

	for (run = 0; run < test_sched_fork_runs; run++)
		so_fork(test_sched_handler_10_worker, 0);
}

void test_sched_10(void)
{
	test_sched_fork_runs = 5;
	// test_sched_fork_runs = get_rand(1, SO_MAX_UNITS);

	if (so_init(SO_MAX_UNITS, 0) < 0)
	{
		goto test;
	}

	so_fork(test_sched_handler_10_master, SO_MAX_PRIO);
	sched_yield();

test:
	so_end();

	printf("%u %u\n", test_sched_fork_handler_runs, test_sched_fork_runs);
}

static unsigned int test_sched_fork_executed;

static void test_sched_handler_07(unsigned int prio)
{
	test_sched_fork_executed = 1;
}

void test_sched_07(void)
{
	test_sched_fork_executed = 0;

	if (so_init(SO_MAX_UNITS, 0) < 0)
	{
		goto test;
	}

	/* invalid handler */
	if (so_fork(test_sched_handler_07, 0) == INVALID_TID)
	{
		goto test;
	}

test:
	so_end();

	printf("%u\n", test_sched_fork_executed);
}

static tid_t test_exec_last_tid;
static tid_t test_tid_13_1;
static tid_t test_tid_13_2;
static unsigned int test_exec_status;
static unsigned int test_exec_nr;
static tid_t test_exec_tid;

#define SO_TEST_AND_SET(expect_id, new_id)                \
	do                                                    \
	{                                                     \
		if (equal_tids((expect_id), INVALID_TID) ||       \
			equal_tids((new_id), INVALID_TID))            \
			so_fail("invalid task id");                   \
		if (!equal_tids(test_exec_last_tid, (expect_id))) \
			so_fail("invalid tasks order");               \
		test_exec_last_tid = (new_id);                    \
	} while (0)

#define SO_SET(expect_id, new_id)      \
	do                                 \
	{                                  \
		test_exec_last_tid = (new_id); \
	} while (0)

static void test_sched_handler_13_2(unsigned int dummy)
{
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	test_exec_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_13_1(unsigned int dummy)
{
	test_exec_last_tid = test_tid_13_1 = get_tid();
	test_tid_13_2 = so_fork(test_sched_handler_13_2, 0);

	/* allow the other thread to init */
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_1);
	so_exec();

	/* make sure nobody changed it until now */
	test_exec_status = SO_TEST_FAIL;
}

void test_sched_13(void)
{
	test_exec_status = SO_TEST_FAIL;

	/* quantum is 2, so each task should be preempted
	 * after running two instructions
	 */
	so_init(2, 0);

	so_fork(test_sched_handler_13_1, 0);

	sched_yield();
	so_end();

	printf("%u\n", test_exec_status);
}

static unsigned int test_fork_rand_tests;
static unsigned int test_fork_execution_status;
static tid_t test_fork_exec_tids[SO_MAX_UNITS];
static tid_t test_fork_tids[SO_MAX_UNITS];

static void test_sched_handler_11_worker(unsigned int dummy)
{
	static unsigned int exec_idx;

	/* signal that he's the one that executes in round exec_idx */
	test_fork_exec_tids[exec_idx++] = get_tid();
	test_fork_execution_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_11_master(unsigned int dummy)
{
	unsigned int i;

	/*
	 * this thread should not be preempted as it executes maximum
	 * SO_MAX_UNITS - 1, and the quantum time is SO_MAX_UNITS
	 */
	/* use a cannary value to detect overflow */
	test_fork_tids[test_fork_rand_tests] = get_tid();
	test_fork_exec_tids[test_fork_rand_tests] = get_tid();
	for (i = 0; i < test_fork_rand_tests; i++)
	{
		test_fork_tids[i] = so_fork(test_sched_handler_11_worker, 0);
	}
}

void basic_test(unsigned int a)
{
	printf("%u\n", a);
}

void test_sched_11(void)
{
	unsigned int i;

	test_fork_rand_tests = get_rand(1, SO_MAX_UNITS - 1);
	test_fork_execution_status = SO_TEST_FAIL;

	so_init(SO_MAX_UNITS, 0);

	so_fork(test_sched_handler_11_master, 0);

	sched_yield();
	so_end();

	printf("\n");
	if (test_fork_execution_status == SO_TEST_SUCCESS)
	{
		/* check threads order */
		for (i = 0; i <= test_fork_rand_tests; i++)
		{
			printf("%ld %ld\n", test_fork_tids[i], test_fork_exec_tids[i]);
			if (!equal_tids(test_fork_exec_tids[i],
							test_fork_tids[i]))
			{
				so_error("different thread ids");
				test_fork_execution_status = SO_TEST_FAIL;
				break;
			}
		}
	}
	else
	{
		so_error("threads execution failed");
	}

	basic_test(test_fork_execution_status);
}

static tid_t test_tid_14_1;
static tid_t test_tid_14_2;
static tid_t test_tid_14_3;
static tid_t test_tid_14_4;

static void test_sched_handler_14_4(unsigned int dummy)
{
	printf("tid4 %ld\n", get_tid());
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_4);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_4, test_tid_14_4);

	test_exec_status = SO_TEST_FAIL;
}

static void test_sched_handler_14_3(unsigned int dummy)
{
	printf("tid3 %ld\n", get_tid());
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	/* should be preempted here */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);

	/* done scheduling */
	test_exec_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_14_2(unsigned int dummy)
{
	printf("tid2 %ld\n", get_tid());
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_2);

	/* leaving - thread 3 should start */
	test_exec_status = SO_TEST_FAIL;
}

static void test_sched_handler_14_1(unsigned int dummy)
{
	printf("tid1 %ld\n", get_tid());
	test_exec_last_tid = test_tid_14_1 = get_tid();
	test_tid_14_2 = so_fork(test_sched_handler_14_2, 0);
	/* allow the other thread to init */
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	test_tid_14_3 = so_fork(test_sched_handler_14_3, 0);
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	test_tid_14_4 = so_fork(test_sched_handler_14_4, 0);
	sched_yield();

	/* still me */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	so_exec();

	/* should be preempted here */
	SO_TEST_AND_SET(test_tid_14_4, test_tid_14_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);

	/* leaving - make sure the others execute successfully */
	test_exec_status = SO_TEST_FAIL;
}

void test_sched_14(void)
{
	test_exec_status = SO_TEST_FAIL;

	so_init(4, 0);

	so_fork(test_sched_handler_14_1, 0);

	sched_yield();
	so_end();

	basic_test(test_exec_status);
}

static void test_sched_handler_20_signal(unsigned int dummy)
{
	unsigned int step;

	/* check if wait was called */
	if (exec_time != 2) {
		so_error("thread didn't execute wait");
		return;
	}

	/* keep the other thread waiting longer than the preemption time */
	for (step = 0; step <= SO_PREEMPT_UNITS; step++) {
		exec_time++;
		so_exec();
	}

	/* if executed before signal, fail */
	if (test_exec_status == SO_TEST_SUCCESS)
		test_exec_status = SO_TEST_FAIL;
	/* finally release the waiting thread */
	so_signal(SO_DEV0);
}

static void test_sched_handler_20_wait(unsigned int dummy)
{
	exec_time++;
	so_fork(test_sched_handler_20_signal, 0);
	exec_time++;
	so_wait(SO_DEV0);

	/* check if I waited more than preemption time */
	if (exec_time < SO_PREEMPT_UNITS + 2) {
		so_error("scheduled while waiting");
		return;
	}

	so_exec();
	test_exec_status = SO_TEST_SUCCESS;
}

/* tests the IO functionality */
void test_sched_20(void)
{
	/* ensure that the thread gets to execute wait */
	so_init(SO_PREEMPT_UNITS, 1);

	so_fork(test_sched_handler_20_wait, 0);

	sched_yield();
	so_end();

	basic_test(test_exec_status);
}

#define FAIL_IF_NOT_PRIO(prio, msg) \
	do { \
		if ((prio) != last_priority) { \
			test_exec_status = SO_TEST_FAIL; \
			so_fail(msg); \
		} \
		last_priority = priority; \
	} while (0)

/*
 * Threads are mixed to wait/signal lower/higher priorities
 * P2 refers to the task with priority 2
 */
static void test_sched_handler_21(unsigned int priority)
{
	switch (priority) {
	case 1:
		/* P2 should be the one that executed last */
		FAIL_IF_NOT_PRIO(2, "should have been woke by P2");
		if (so_signal(SO_DEV3) == 0)
			so_fail("dev3 does not exist");
		so_exec();
		FAIL_IF_NOT_PRIO(1, "invalid preemption");
		if (so_signal(SO_DEV0) != 2)
			so_fail("P1 should wake P3 and P4 (dev0)");
		FAIL_IF_NOT_PRIO(2, "preempted too early");
		if (so_signal(SO_DEV1) != 1)
			so_fail("P1 should wake P3 (dev1)");
		FAIL_IF_NOT_PRIO(2, "woke by someone else");
		if (so_signal(SO_DEV0) != 1)
			so_fail("P1 should wake P4 (dev0)");
		FAIL_IF_NOT_PRIO(4, "should be the last one");
		so_exec();
		FAIL_IF_NOT_PRIO(1, "someone else was running");
		break;

	case 2:
		last_priority = 2;
		/* wait for dev 3 - invalid device */
		if (so_wait(SO_DEV3) == 0)
			so_fail("dev3 does not exist");
		/* spawn all the tasks */
		so_fork(test_sched_handler_21, 4);
		so_fork(test_sched_handler_21, 3);
		so_fork(test_sched_handler_21, 1);
		so_exec();
		so_exec();

		/* no one should have ran until now */
		FAIL_IF_NOT_PRIO(2, "somebody else ran before P2");
		if (so_wait(SO_DEV1) != 0)
			so_fail("cannot wait on dev1");
		FAIL_IF_NOT_PRIO(3, "should run after P3");
		if (so_wait(SO_DEV2) != 0)
			so_fail("cannot wait on dev2");
		FAIL_IF_NOT_PRIO(3, "only P3 could wake me");
		so_exec();
		break;

	case 3:
		if (so_wait(SO_DEV0) != 0)
			so_fail("P3 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(4, "priority order violated");
		if (so_wait(SO_DEV1) != 0)
			so_fail("P3 cannot wait on dev1");
		FAIL_IF_NOT_PRIO(1, "someone else woke P3");
		if (so_signal(SO_DEV2) != 1)
			so_fail("P3 should wake P2 (dev2)");
		break;

	case 4:
		if (so_wait(SO_DEV0) != 0)
			so_fail("P4 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(1, "lower priority violation");
		if (so_signal(SO_DEV1) != 1)
			so_fail("P4 should wake P2 (dev1)");
		if (so_wait(SO_DEV0) != 0)
			so_fail("P4 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(1, "someone else woke dev0");
		break;
	}
}

/* tests the IO and priorities */
void test_sched_21(void)
{
	test_exec_status = SO_TEST_SUCCESS;

	so_init(1, 3);

	so_fork(test_sched_handler_21, 2);

	sched_yield();
	so_end();

	basic_test(test_exec_status == SO_TEST_SUCCESS && last_priority == 1);
}


int main()
{
	test_sched_21();
}