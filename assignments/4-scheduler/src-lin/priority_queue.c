#include "priority_queue.h"
#include "linked_list.h"

PriorityQueue *init_prio_queue(void)
{
	int priority;
	PriorityQueue *prio_queue;

	prio_queue = malloc(sizeof(PriorityQueue));
	for (priority = 0; priority <= SO_MAX_PRIO; priority++) {
		prio_queue->priority_lists[priority] = init_list();
		if (!prio_queue->priority_lists[priority])
			return NULL;
	}

	return prio_queue;
}

ThreadInfo add_to_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority, tid_t curr_thread_id)
{
	return add_to_list(prio_queue->priority_lists[priority], thread_id, curr_thread_id);
}

void append_node_to_queue(PriorityQueue *prio_queue, LinkedListNode *node, unsigned int priority)
{
	append_node(prio_queue->priority_lists[priority], node);
}

int delete_from_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority)
{
	return delete_from_list(prio_queue->priority_lists[priority], thread_id);
}

LinkedListNode *remove_from_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority)
{
	return remove_from_list(prio_queue->priority_lists[priority], thread_id);
}

ThreadInfo get_next_thread(PriorityQueue *prio_queue)
{
	int priority;
	LinkedList *curr_list;
	ThreadInfo invalid_thread_info;

	/* parcurge listele de prioritati pana gaseste una nevida */
	/* intoarce capul din aceasta */
	for (priority = SO_MAX_PRIO; priority >= 0; priority--) {
		curr_list = prio_queue->priority_lists[priority];
		if (curr_list->size == 0)
			continue;

		return get_first_from_list(curr_list);
	}

	/* daca nu e nicio lista nevida intoarce eroare */
	invalid_thread_info.id = INVALID_TID;
	return invalid_thread_info;
}

unsigned int get_highest_priority(PriorityQueue *prio_queue)
{
	int priority;

	for (priority = SO_MAX_PRIO; priority >= 0; priority--)
		if (prio_queue->priority_lists[priority]->size != 0)
			return priority;

	return 0;
}

void free_prio_queue(PriorityQueue *prio_queue)
{
	int priority;

	for (priority = 0; priority <= SO_MAX_PRIO; priority++)
		free_list(prio_queue->priority_lists[priority]);
	
	free(prio_queue);
}

void print_prio_queue(PriorityQueue *prio_queue)
{
	int priority;

	for (priority = 0; priority <= SO_MAX_PRIO; priority++) {
		printf("%d: ", priority);
		print_list(prio_queue->priority_lists[priority]);
	}

	printf("\n");
}
