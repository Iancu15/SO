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
	/* variabila de conditie folosita pentru a implementa so_wait */
	/* si so_signal, alaturi de mutex-ul ei */
	pthread_mutex_t mutex;
	pthread_cond_t cond;

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
	sem_t *start_thread_sem_1;
	sem_t *start_thread_sem_2;
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

/* verifica daca thread-ul curent trebuie preemptat */
static int check_scheduler(void)
{
	/* incrementeaza contorul thread-ului si verifica daca a depasit cuanta */
	/* de timp alocata */
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
		r = pthread_mutex_init(&(sched->event_list[i].mutex), NULL);
		if (r < 0)
			return FAILURE;

		r = pthread_cond_init(&(sched->event_list[i].cond), NULL);
		if (r < 0)
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

	for (i = 0; i < sched->number_of_events; i++) {
		r = pthread_mutex_destroy(&(sched->event_list[i].mutex));
		if (r < 0)
			return FAILURE;

		r = pthread_cond_destroy(&(sched->event_list[i].cond));
		if (r < 0)
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

	sched->start_thread_sem_1 = malloc(sizeof(sem_t));
	if (!sched->start_thread_sem_1)
		return FAILURE;

	r = sem_init(sched->start_thread_sem_1, 0, 0);
	if (r < 0)
		return FAILURE;

	sched->start_thread_sem_2 = malloc(sizeof(sem_t));
	if (!sched->start_thread_sem_2)
		return FAILURE;

	r = sem_init(sched->start_thread_sem_2, 0, 0);
	if (r < 0)
		return FAILURE;

	if (init_event_list(io) < 0)
		return FAILURE;

	is_sched_init = SCHED_IS_INIT;

	return SUCCESS;
}

static void *start_thread(void *params)
{
	ThreadArg thread_arg;
	ThreadInfo my_thread_info;
	int r;

	/* anunt thread-ul parinte ca copilul a intrat in starea READY/RUNNING */
	thread_arg = *(ThreadArg *)params;
	r = sem_post(sched->start_thread_sem_1);
	if (r < 0)
		exit(EXIT_FAILURE);

	/* thread-ul asteapta ca thread-ul parinte sa modifice new_thread_info */
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
	r = delete_from_prio_queue(sched->queue, my_thread_info.id, thread_arg.priority);
	if (r < 0)
		exit(EXIT_FAILURE);

	r = yield(YIELD_WITHOUT_WAIT);
	if (r < 0)
		exit(EXIT_FAILURE);

	return NULL;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	pthread_t thread_id;
	ThreadArg thread_arg;
	int r;

	if (priority > SO_MAX_PRIO || !func)
		return INVALID_TID;

	thread_arg.func = func;
	thread_arg.priority = priority;
	r = pthread_create(&thread_id, NULL, start_thread, (void *)&thread_arg);
	if (r < 0) {
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
	/* salvez in variabila new_thread_info informatii legate de thread pe */
	/* care acesta le poate folosi */
	sched->new_thread_info = add_to_prio_queue(sched->queue, thread_id, priority, sched->curr_thread_info.id);
	if (sched->new_thread_info.id == INVALID_TID) {
		sched->curr_time_on_proc++;
		return INVALID_TID;
	}

	if (add_to_list(sched->thread_list, thread_id, UNUSED_CURR_THREAD).id == INVALID_TID) {
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
	/* daca este root, atunci acesta doar trezeste thread-ul nou creat si */
	/* isi vede de treaba */
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
	LinkedListNode *my_thread_node;
	unsigned int my_priority;
	int r;

	if (io >= sched->number_of_events) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	r = pthread_mutex_lock(&(sched->event_list[io].mutex));
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* scot thread-ul din coada de prioritati ca va intra in starea */
	/* de WAITING */
	my_priority = sched->curr_thread_priority;
	my_thread_node = remove_from_prio_queue(sched->queue, sched->curr_thread_info.id, sched->curr_thread_priority);
	if (!my_thread_node) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* il adaug in lista cu thread-uri in asteptare asociata evenimentului */
	/* dupa care thread-ul asteapta */
	my_thread_node->thread_info.priority = my_priority;
	append_node(sched->event_list[io].waiting_list, my_thread_node);
	r = yield(YIELD_WITHOUT_WAIT);
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* astept sa aiba loc evenimentul I/O */
	r = pthread_cond_wait(&(sched->event_list[io].cond), &(sched->event_list[io].mutex));
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	r = pthread_mutex_unlock(&(sched->event_list[io].mutex));
	if (r < 0) {
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

	list = sched->event_list[io].waiting_list;
	size = list->size;
	if (size == 0)
		return 0;

	curr_node = list->start;
	for (i = 0; i < size; i++) {
		next_node = curr_node->next;
		append_node_to_queue(sched->queue, curr_node, curr_node->thread_info.priority);
		curr_node = next_node;
	}

	clear_list(list);

	return size;
}

int so_signal(unsigned int io)
{
	int r, number_of_woken_threads;

	if (io >= sched->number_of_events) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	r = pthread_mutex_lock(&(sched->event_list[io].mutex));
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	/* semnaleaza toate thread-urile care fac wait dupa eveniment */
	/* ca pot iesi din wait */
	r = pthread_cond_broadcast(&(sched->event_list[io].cond));
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

	r = pthread_mutex_unlock(&(sched->event_list[io].mutex));
	if (r < 0) {
		sched->curr_time_on_proc++;
		return FAILURE;
	}

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

	if (sched->thread_list->size == 0)
		return SUCCESS;

	curr_node = sched->thread_list->start;
	do {
		if (pthread_join(curr_node->thread_info.id, NULL) < 0)
			return FAILURE;

		curr_node = curr_node->next;
	} while (curr_node != sched->thread_list->start);

	return SUCCESS;
}

void so_end(void)
{
	int r;

	if (is_sched_init == SCHED_ISNT_INIT)
		return;

	r = sched_join_threads();
	if (r < 0)
		exit(EXIT_FAILURE);

	free_prio_queue(sched->queue);
	free_list(sched->thread_list);
	r = sem_destroy(sched->start_thread_sem_1);
	if (r < 0)
		exit(EXIT_FAILURE);

	r = sem_destroy(sched->start_thread_sem_2);
	if (r < 0)
		exit(EXIT_FAILURE);

	free(sched->start_thread_sem_1);
	free(sched->start_thread_sem_2);
	r = free_event_list();
	if (r < 0)
		exit(EXIT_FAILURE);

	free(sched);
	is_sched_init = SCHED_ISNT_INIT;
}
