#include "linked_list.h"

LinkedList *init_list(void)
{
    LinkedList *list;

    list = malloc(sizeof(LinkedList));
    if (!list)
        return NULL;

    list->size = 0;
    list->start = NULL;
    list->end = NULL;

    return list;
}

void clear_list(LinkedList *list)
{
    list->end = NULL;
    list->start = NULL;
    list->size = 0;
}

void append_node(LinkedList *list, LinkedListNode *node)
{
    if (list->size == 0) {
        node->next = node;
        list->start = node;
        list->end = node;
        list->size = 1;
    } else {
        list->end->next = node;
        node->next = list->start;
        list->end = node;
        list->size++;
    }
}

ThreadInfo add_to_list(LinkedList *list, tid_t thread_id, tid_t curr_thread_id)
{
    int r;
    LinkedListNode *node, *curr_node;
    ThreadInfo invalid_thread_info;

    if (!list) {
        invalid_thread_info.id = INVALID_TID;
        return invalid_thread_info;
    }

    node = malloc(sizeof(LinkedListNode));
    if (!node) {
        invalid_thread_info.id = INVALID_TID;
        return invalid_thread_info;
    }

    node->thread_info.id = thread_id;
    node->thread_info.sem = malloc(sizeof(sem_t));
    if (!node->thread_info.sem) {
        invalid_thread_info.id = INVALID_TID;
        return invalid_thread_info;
    }

    /* initializez un semafor binar cu valoarea initiala 0 */
    r = sem_init(node->thread_info.sem, 0, 0);
    if (r < 0) {
        invalid_thread_info.id = INVALID_TID;
        return invalid_thread_info;
    }

    if (list->size == 0) {
        node->next = node;
        list->start = node;
        list->end = node;
        list->size = 1;
    } else {
        /* daca coada listei are id-ul curr_thread_id, elementul se pune pe */
        /* antepenultima pozitie */
        if (curr_thread_id != UNUSED_CURR_THREAD && list->end->thread_info.id == curr_thread_id) {
            curr_node = list->start;

            /* se cauta antepenultim element */
            while (curr_node->next != list->end)
                curr_node = curr_node->next;

            /* se pune dupa antepenultimul element si inainte de ultimul element */
            curr_node->next = node;
            node->next = list->end;
            if (list->size == 1)
                list->start = node;

            list->size++;
        } else {
            /* altfel se pune la final */
            list->end->next = node;
            node->next = list->start;
            list->end = node;
            list->size++;
        }
    }

    return node->thread_info;
}

int free_node(LinkedListNode *node)
{
    if (sem_destroy(node->thread_info.sem) < 0)
        return FAILURE;

    free(node->thread_info.sem);
    free(node);
    return SUCCESS;
}

int delete_from_list(LinkedList *list, tid_t thread_id)
{
    LinkedListNode *removed_node;

    removed_node = remove_from_list(list, thread_id);
    if (!removed_node)
        return FAILURE;

    return free_node(removed_node);
}

LinkedListNode *remove_from_list(LinkedList *list, tid_t thread_id)
{
    LinkedListNode *curr_node;
    LinkedListNode *temp;

    temp = NULL;
    if (!list || list->size == 0)
        return NULL;

    if (list->size == 1) {
        if (list->start->thread_info.id != thread_id)
            return NULL;

        temp = list->start;
        list->start = NULL;
        list->end = NULL;
    } else if (list->start->thread_info.id == thread_id) {
        temp = list->start;

        list->end->next = list->start->next;
        list->start = list->start->next;
    } else {
        curr_node = list->start;

        /* caut nodul precedent celui ce trebuie eliminat */
        while (curr_node->next->thread_info.id != thread_id) {
            if (curr_node->next == list->start)
                return NULL;

            curr_node = curr_node->next;
        }

        temp = curr_node->next;
        curr_node->next = temp->next;
        if (temp == list->end)
            list->end = curr_node;
    }

    list->size--;

    return temp;
}

/* shifteaza lista la stanga, vechiul start devine noul end si noul */
/* start e al doilea element din lista */
static void shift_list(LinkedList *list)
{
    list->start = list->start->next;
    list->end = list->end->next;
}

ThreadInfo get_first_from_list(LinkedList *list)
{
    ThreadInfo thread_info;

    if (!list || list->size == 0) {
        thread_info.id = INVALID_TID;
        return thread_info;
    }

    thread_info = list->start->thread_info;
    shift_list(list);

    return thread_info;
}

void free_list(LinkedList *list)
{
    LinkedListNode *curr_node;
    LinkedListNode *temp;
    int r;

    if (!list)
        return;

    if (list->size == 0) {
        free(list);
        return;
    }

    curr_node = list->start;
    while (curr_node != list->end) {
        temp = curr_node;
        curr_node = curr_node->next;
        r = free_node(temp);
        if (r < 0)
            exit(EXIT_FAILURE);
    }

    r = free_node(list->end);
    if (r < 0)
        exit(EXIT_FAILURE);

    free(list);
}

void print_list(LinkedList *list)
{
    LinkedListNode *curr_node;

    if (!list)
        return;

    if (list->size == 0) {
        printf("LIST IS EMPTY\n");
        return;
    }

    curr_node = list->start;
    do {
        printf("%ld ", curr_node->thread_info.id);
        curr_node = curr_node->next;
    } while (curr_node != list->start);

    printf("\\s:%ld ", list->start->thread_info.id);
    printf("\\e:%ld ", list->end->thread_info.id);
    printf("size:%d", list->size);
    printf("\n");
}
