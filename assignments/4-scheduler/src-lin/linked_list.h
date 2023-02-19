#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "util/so_scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

/* informatiile relevante thread-ului stocat in lista */
typedef struct {
	tid_t id;

	/* un semafor binar folosit pentru sincronizarea intre thread-uri */
	sem_t *sem;
	unsigned int priority;
} ThreadInfo;

typedef struct node {
	struct node *next;
	ThreadInfo thread_info;
} LinkedListNode;

typedef struct {
	LinkedListNode *start;
	LinkedListNode *end;
	unsigned int size;
} LinkedList;

/* folosite pentru valoarea de return la functiile ce intorc int */
#define FAILURE -1
#define SUCCESS 0

/* poate fi dat ca al treilea argument la add_to_list si semnifica ca se */
/* poate ignora in operatie campul respectiv */
#define UNUSED_CURR_THREAD 0

/* initializeaza lista simplu inlantuita */
/* intoarce NULL in caz de eroare */
LinkedList *init_list(void);

/* goleste lista fara sa elibereze memoria pentru noduri */
void clear_list(LinkedList *list);

/* adauga un thread la finalul listei, daca curr_thread_id se afla la */
/* finalul listei, atunci thread-ul va fi pus pe penultima pozitie */
/* se aloca toate structurile aferente nodului */
/* intoarce un ThreadInfo cu thread_id-ul setat la INVALID_TID in caz */
/* de eroare */
ThreadInfo add_to_list(LinkedList *list, tid_t thread_id, tid_t curr_thread_id);

/* se adauga un nod la finalul listei, nu se mai aloca alte structuri */
void append_node(LinkedList *list, LinkedListNode *node);

/* se sterge un thread din lista daca exista si i se elibereaza resursele */
/* intoarce -1 in caz de eroare sau daca nu s-a gasit thread-ul */
int delete_from_list(LinkedList *list, tid_t thread_id);

/* se sterge un thread din lista fara sa i se elibereze resursele si este */
/* returnat, in caz de eroare sau daca nu s-a gasit nodul respectiv se */
/* intoarce NULL */
LinkedListNode *remove_from_list(LinkedList *list, tid_t thread_id);

/* intoarce capul listei si shifteaza lista la stanga */
/* intoarce un ThreadInfo cu thread_id-ul setat la INVALID_TID in caz */
/* de eroare */
ThreadInfo get_first_from_list(LinkedList *list);

/* elibereaza lista si toate nodurile acesteia */
void free_list(LinkedList *list);

void print_list(LinkedList *list);

#endif
