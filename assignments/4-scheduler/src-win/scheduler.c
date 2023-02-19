#include "util/so_scheduler.h"
#include "priority_queue.h"
#include <stdlib.h>

/* argumentul thread-ului dat catre start_thread de create_thread */
typedef struct {
	so_handler *func;
	unsigned int priority;
} ThreadArg;

/* structura unui eveniment I/O */
typedef struct {
	/* folosit pentru sincronizarea intre evenimente */
	HANDLE hEvent;

	/* lista de thread-uri aflate in asteptare pe evenimentul respectiv */
	LinkedList *waiting_list;
} Event;

/* structura ce contine toate informatiile necesare planificatorului */
typedef struct {
	/* boolean setat pana ce thread-ul root face so_fork la alt thread */
	unsigned int is_root_thread;

	/* cuanta de timp admisa pe procesor */
	unsigned int max_time_on_proc;

	/* timpul petrecut pe procesor de thread-ul curent planificat */
	unsigned int curr_time_on_proc;

	/* numarul de evenimente I/O din rulare */
	unsigned int number_of_events;

	/* informatiile thread-ului curent planificat pe procesor */
	ThreadInfo curr_thread_info;
	unsigned int curr_thread_priority;

	/* informatiile thread-ului nou creat prin so_fork */
	ThreadInfo new_thread_info;

	/* 2 semafoare ce sincronizeaza so_fork si start_thread */
	HANDLE start_thread_sem_1;
	HANDLE start_thread_sem_2;
	PriorityQueue *queue;

	/* istoricul cu thread-urile ce au fost create si id-urile lor */
	LinkedList *thread_list;

	/* lista cu structurile evenimentelor I/O din rulare */
	Event *event_list;
} Scheduler;

/* instanta planificatorului */
static Scheduler *sched;

#define SCHED_IS_INIT 1
#define SCHED_ISNT_INIT -1

/* bool that says whether the scheduler is initialized */
static int is_sched_init = SCHED_ISNT_INIT;

/* folosit in check_scheduler, IS_PREEMPTED inseamna ca thread-ul curent */
/* trebuie scos de pe procesor, nu trebuie scos in cazul IS_NOT_PREEMPTED */
#define IS_PREEMPTED 1
#define IS_NOT_PREEMPTED 0

/* folosit ca argument pentru functia yield */
/* YIELD_AND_WAIT inseamna ca face yield catre alt thread si asteapta sa fie */
/* replanificat */
/* YIELD_WITHOUT_WAIT inseamna ca face yield catre alt thread si nu asteapta */
/* sa fie replanificat */
#define YIELD_AND_WAIT 1
#define YIELD_WITHOUT_WAIT 0

/* decrementeaza valoarea semaforului */
/* daca valoarea e 0 face wait pana e incrementata */
/* intoarce 0 in caz de succes si -1 in caz de eroare */
static int sem_wait(HANDLE sem)
{
	DWORD dwRet;

	dwRet = WaitForSingleObject(
		sem,
		INFINITE
	);

	if (dwRet == WAIT_FAILED)
		return FAILURE;

	return SUCCESS;
}

/* incrementeaza valoarea semaforului */
/* intoarce 0 in caz de succes si -1 in caz de eroare */
static int sem_post(HANDLE sem)
{
	BOOL bRet;

	bRet = ReleaseSemaphore(
		sem,
		1,
		NULL
	);

	if (bRet == FALSE)
		return FAILURE;

	return SUCCESS;
}

/* verifica daca thread-ul curent trebuie preemptat */
static int check_scheduler(void)
{
	/* incrementeaza contorul thread-ului si verifica daca */
	/* a depasit cuanta de timp alocata */
	sched->curr_time_on_proc++;
	if (sched->curr_time_on_proc == sched->max_time_on_proc)
		return IS_PREEMPTED;

	/* verifica daca a aparut un thread cu prioritate mai mare in sistem */
	if (sched->curr_thread_priority < get_highest_priority(sched->queue))
		return IS_PREEMPTED;

	return IS_NOT_PREEMPTED;
}

/* thread-ul de pe procesor cedeaza locul altui thread */
static int yield(int type)
{
	int r;
	ThreadInfo next_thread_info, my_thread_info;
	unsigned int my_priority;

	next_thread_info = get_next_thread(sched->queue);
	if (type != YIELD_AND_WAIT && type != YIELD_WITHOUT_WAIT)
		return FAILURE;

	/* nu mai sunt thread-uri de planificat */
	if (next_thread_info.id == INVALID_TID)
		return SUCCESS;

	/* trezesc thread-ul ce este planificat in locul acestuia */
	my_thread_info = sched->curr_thread_info;
	my_priority = sched->curr_thread_priority;
	r = sem_post(next_thread_info.sem);
	if (r < 0)
		return FAILURE;

	if (type == YIELD_AND_WAIT) {
		/* thread-ul asteapta sa fie replanificat */
		r = sem_wait(my_thread_info.sem);
		if (r < 0)
			return FAILURE;

		sched->curr_thread_info = my_thread_info;
		sched->curr_thread_priority = my_priority;
		sched->curr_time_on_proc = 0;
	}

	return SUCCESS;
}

static int init_event_list(unsigned int io)
{
	int i, r;

	sched->event_list = malloc(io * sizeof(Event));
	if (!sched->event_list)
		return FAILURE;

	for (i = 0; i < io; i++) {
		sched->event_list[i].hEvent = CreateEvent(
			NULL,
			TRUE,
			TRUE,
			NULL
		);

		if (sched->event_list[i].hEvent == INVALID_HANDLE_VALUE)
			return FAILURE;

		sched->event_list[i].waiting_list = init_list();
		if (!sched->event_list[i].waiting_list)
			return FAILURE;
	}

	return SUCCESS;
}

static int free_event_list(void)
{
	int i, r;
	BOOL bRet;

	for (i = 0; i < sched->number_of_events; i++) {
		bRet = CloseHandle(sched->event_list[i].hEvent);
		if (bRet == FALSE)
			return FAILURE;

		free_list(sched->event_list[i].waiting_list);
	}

	free(sched->event_list);
	return SUCCESS;
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	int r;

	if (io > SO_MAX_NUM_EVENTS || time_quantum == 0)
		return FAILURE;

	if (is_sched_init == SCHED_IS_INIT)
		return FAILURE;

	sched = malloc(sizeof(Scheduler));
	if (!sched)
		return FAILURE;

	sched->is_root_thread = 1;
	sched->curr_time_on_proc = 0;
	sched->max_time_on_proc = time_quantum;
	sched->number_of_events = io;
	sched->queue = init_prio_queue();
	if (!sched->queue)
		return FAILURE;

	sched->thread_list = init_list();
	if (!sched->thread_list)
		return FAILURE;

	sched->start_thread_sem_1 = CreateSemaphore(
		NULL,
		0,
		1,
		NULL
	);

	if (sched->start_thread_sem_1 == INVALID_HANDLE_VALUE)
		return FAILURE;

	sched->start_thread_sem_2 = CreateSemaphore(
		NULL,
		0,
		1,
		NULL
	);

	if (sched->start_thread_sem_2 == INVALID_HANDLE_VALUE)
		return FAILURE;

	if (init_event_list(io) < 0)
		return FAILURE;

	is_sched_init = SCHED_IS_INIT;

	return SUCCESS;
}

static DWORD WINAPI start_thread(LPVOID params)
{
	ThreadArg thread_arg;
	ThreadInfo my_thread_info;
	tid_t my_tid;
	int r;

	/* anunt thread-ul parinte ca copilul a intrat */
	/* in starea READY/RUNNING */
	thread_arg = *(ThreadArg *)params;
	r = sem_post(sched->start_thread_sem_1);
	if (r < 0)
		exit(EXIT_FAILURE);

	/* thread-ul asteapta ca thread-ul parinte */
	/* sa modifice new_thread_info */
	r = sem_wait(sched->start_thread_sem_2);
	if (r < 0)
		exit(EXIT_FAILURE);

	/* thread-ul copil asteapta sa fie planificat */
	my_thread_info = sched->new_thread_info;
	r = sem_wait(my_thread_info.sem);
	if (r < 0)
		exit(EXIT_FAILURE);

	/* apelez handler-ul thread-ului */
	sched->curr_thread_info = my_thread_info;
	sched->curr_thread_priority = thread_arg.priority;
	sched->curr_time_on_proc = 0;
	thread_arg.func(thread_arg.priority);

	/* cand se termina de rulat il sterg din coada de prioritati */
	/* si dau yield pentru a lasa alt thread sa fie planificat */
	my_tid = my_thread_info.id;
	r = delete_from_prio_queue(sched->queue, my_tid, thread_arg.priority);
	if (r < 0)
		exit(EXIT_FAILURE);

	r = yield(YIELD_WITHOUT_WAIT);
	if (r < 0)
		exit(EXIT_FAILURE);

	return NULL;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	DWORD thread_id;
	HANDLE thread_handle;
	ThreadArg thread_arg;
	tid_t curr_tid;
	ThreadInfo new_tinfo;
	tid_t add_tid;
	PriorityQueue *queue;
	int r;

	if (priority > SO_MAX_PRIO || !func)
		return INVALID_TID;

	thread_arg.func = func;
	thread_arg.priority = priority;
	thread_handle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) start_thread,
		(LPVOID) &thread_arg,
		0,
		&thread_id
	);

	if (thread_handle == INVALID_HANDLE_VALUE) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	/* astept ca thread-ul nou creat sa intre in starea READY/RUNNING */
	r = sem_wait(sched->start_thread_sem_1);
	if (r < 0) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	/* adaug thread-ul in coada de prioritati si in istoric */
	/* salvez in variabila new_thread_info informatii legate */
	/* de thread pe care acesta le poate folosi */
	curr_tid = sched->curr_thread_info.id;
	queue = sched->queue;
	new_tinfo = add_to_prio_queue(queue, thread_id, priority, curr_tid);
	sched->new_thread_info = new_tinfo;
	if (sched->new_thread_info.id == INVALID_TID) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	curr_tid = UNUSED_CURR_THREAD;
	add_tid = add_to_list(sched->thread_list, thread_id, curr_tid).id;
	if (add_tid == INVALID_TID) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	/* trezesc thread-ul dupa ce am iesit din regiunea critica in care */
	/* am modificat new_thread_info */
	r = sem_post(sched->start_thread_sem_2);
	if (r < 0) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	/* daca thread-ul nu e root se fac verificarile uzuale */
	/* daca este root, atunci acesta doar trezeste thread-ul */
	/* nou creat si isi vede de treaba */
	if (!sched->is_root_thread) {
		if (check_scheduler() == IS_PREEMPTED) {
			r = yield(YIELD_AND_WAIT);
			if (r < 0)
				return INVALID_TID;
		}
	} else {
		sched->is_root_thread = 0;
		r = sem_post(sched->new_thread_info.sem);
		if (r < 0) {
			sched->curr_time_on_proc++;
			return INVALID_TID;
		}
	}

	return thread_id;
}

int so_wait(unsigned int io)
{
	LinkedListNode *my_thread_node, *my_node;
	unsigned int my_priority;
	int r;
	DWORD dwRet;
	BOOL bRet;
	tid_t curr_tid;
	unsigned int curr_prio;

	if (io >= sched->number_of_events) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* scot thread-ul din coada de prioritati ca va intra in starea */
	/* de WAITING */
	my_priority = sched->curr_thread_priority;
	curr_tid = sched->curr_thread_info.id;
	curr_prio = sched->curr_thread_priority;
	my_node = remove_from_prio_queue(sched->queue, curr_tid, curr_prio);
	my_thread_node = my_node;
	if (!my_thread_node) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* il adaug in lista cu thread-uri in asteptare */
	/* asociata evenimentului dupa care thread-ul asteapta */
	my_thread_node->thread_info.priority = my_priority;
	append_node(sched->event_list[io].waiting_list, my_thread_node);
	r = yield(YIELD_WITHOUT_WAIT);
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* resetez event-ul pentru a semnaliza ca sunt */
	/* thread-uri in asteptare pe eveniment */
	bRet = ResetEvent(sched->event_list[io].hEvent);
	if (bRet == FALSE) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* astept sa aiba loc evenimentul I/O */
	dwRet = WaitForSingleObject(
		sched->event_list[io].hEvent,
		INFINITE
	);

	if (dwRet == WAIT_FAILED) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* thread-ul asteapta sa fie replanificat */
	r = sem_wait(my_thread_node->thread_info.sem);
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	sched->curr_thread_info = my_thread_node->thread_info;
	sched->curr_thread_priority = my_priority;
	sched->curr_time_on_proc = 0;

	return SUCCESS;
}

/* baga nodurile care asteaptau dupa evenimentul io inapoi in */
/* coada de prioritati */
static int resume_waiting_threads(unsigned int io)
{
	LinkedListNode *curr_node, *next_node;
	LinkedList *list;
	int i, size;
	unsigned int curr_prio;

	list = sched->event_list[io].waiting_list;
	size = list->size;
	if (size == 0)
		return 0;

	curr_node = list->start;
	for (i = 0; i < size; i++) {
		next_node = curr_node->next;
		curr_prio = curr_node->thread_info.priority;
		append_node_to_queue(sched->queue, curr_node, curr_prio);
		curr_node = next_node;
	}

	clear_list(list);

	return size;
}

int so_signal(unsigned int io)
{
	int r, number_of_woken_threads;
	BOOL bRet;

	if (io >= sched->number_of_events) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* semnaleaza toate thread-urile care fac wait dupa eveniment */
	/* ca pot iesi din wait */
	bRet = PulseEvent(sched->event_list[io].hEvent);
	if (bRet == FALSE)
		return FAILURE;

	number_of_woken_threads = resume_waiting_threads(io);
	if (check_scheduler() == IS_PREEMPTED) {
		r = yield(YIELD_AND_WAIT);
		if (r < 0)
			return FAILURE;
	}

	return number_of_woken_threads;
}

void so_exec(void)
{
	int r;

	do {} while (0);

	if (check_scheduler() == IS_PREEMPTED) {
		r = yield(YIELD_AND_WAIT);
		if (r < 0)
			exit(EXIT_FAILURE);
	}
}

/* face join pe toate thread-urile create */
static int sched_join_threads(void)
{
	LinkedListNode *curr_node;
	HANDLE curr_handle;
	DWORD dwRet;

	if (sched->thread_list->size == 0)
		return SUCCESS;

	curr_node = sched->thread_list->start;
	do {
		curr_handle = OpenThread(
			THREAD_ALL_ACCESS,
			TRUE,
			curr_node->thread_info.id
		);

		if (curr_handle == INVALID_HANDLE_VALUE)
			return FAILURE;

		dwRet = WaitForSingleObject(
			curr_handle,
			INFINITE
		);

		curr_node = curr_node->next;
	} while (curr_node != sched->thread_list->start);

	return SUCCESS;
}

void so_end(void)
{
	int r;
	BOOL bRet;

	if (is_sched_init == SCHED_ISNT_INIT)
		return;

	r = sched_join_threads();
	if (r < 0)
		exit(EXIT_FAILURE);

	free_prio_queue(sched->queue);
	free_list(sched->thread_list);
	bRet = CloseHandle(sched->start_thread_sem_1);
	if (bRet == FALSE)
		return FAILURE;

	bRet = CloseHandle(sched->start_thread_sem_2);
	if (bRet == FALSE)
		return FAILURE;

	r = free_event_list();
	if (r < 0)
		exit(EXIT_FAILURE);

	free(sched);
	is_sched_init = SCHED_ISNT_INIT;
}
