/**
 * @file list.c
 * @brief Implemention of the doubly linked list API.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#include <list.h>

/**
 * @brief Check if the list is empty.
 *
 * @param l Pointer to dummy head.
 *
 * @return 1 if only dummy head is in list, 0 otherwise.
 */
static int list_empty(list *l) {
	return l->prev == l->next;
}

/**
 * @brief Add an entry after prev
 *
 * @param prev Entry after which the new entry is added.
 * @param entry Entry to be added.
 */
static void list_add(list *prev, list *entry) {
	entry->next = prev->next;
	entry->prev = prev;
	prev->next = entry;
}

void list_init(list *head) {
	head->prev = head->next = head;
}
 
list *list_remv(list *entry) {
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	return entry;
}

list *list_remv_head(list *l) {
	if (list_empty(l))
    /* After removing the sole item, the list is empty */
		return NULL;
  /* The first data element is the one after the head as the list is empty
   * when there is only 1 element in it. */
	return list_remv(l->next);
}

void list_add_tail(list *l, list *entry) {
  /* TODO why are there two ->prev? */
	return list_add(l->prev->prev, entry);
}
