#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "linked_list.h"

/* coada de prioritati o simulez prin 6 liste, una pentru fiecare prioritate */
/* daca un thread si nodul sau se afla in lista i, inseamna ca thread-ul are */
/* prioritatea i */
typedef struct {
	LinkedList * priority_lists[SO_MAX_PRIO + 1];
} PriorityQueue;

/* initializez coada si listele aferente */
/* returneaza NULL in caz de eroare */
PriorityQueue *init_prio_queue(void);

/* adaug thread-ul in coada de prioritati si returnez structura creata */
/* pentru acesta care contine semaforul folosit la sincronizare */
/* curr_thread_id este folosit de add_to_list pentru adaugare */
/* intoarce un ThreadInfo cu thread_id-ul setat la INVALID_TID in caz */
/* de eroare */
ThreadInfo add_to_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority, tid_t curr_thread_id);

/* adauga un nod in coada de prioritati, nu se mai aloca noi resurse */
/* pentru thread */
void append_node_to_queue(PriorityQueue *prio_queue, LinkedListNode *node, unsigned int priority);

/* sterg thread-ul din coada de prioritati si ii eliberez resursele */
/* intoarce -1 in caz de eroare sau daca nu s-a gasit thread-ul */
int delete_from_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority);

/* sterg thread-ul din coada de prioritati fara a elibera resursele nodului */
/* intorc nodul sters */
/* in caz de eroare sau daca nu s-a gasit nodul respectiv se intoarce NULL */
LinkedListNode *remove_from_prio_queue(PriorityQueue *prio_queue, tid_t thread_id, unsigned int priority);

/* intoarce structura aferenta thread-ului cu prioritatea cea mai mare din */
/* coada de prioritati */
/* intoarce un ThreadInfo cu thread_id-ul setat la INVALID_TID in caz */
/* de eroare */
ThreadInfo get_next_thread(PriorityQueue *prio_queue);

/* intoarce prioritatea cea mai mare pentru care sunt noduri cu acea */
/* prioritate in coada */
unsigned int get_highest_priority(PriorityQueue *prio_queue);

/* elibereaza coada alaturi de listele sale */
void free_prio_queue(PriorityQueue *prio_queue);

void print_prio_queue(PriorityQueue *prio_queue);

#endif
