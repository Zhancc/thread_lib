/**
 * @file list.c
 * @brief Implemention of the doubly linked list API.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

/* NULL */
#include <stddef.h>

#include <list.h>

/**
 * @brief Check if the list is empty.
 *
 * @param l Pointer to dummy head.
 *
 * @return 1 if only dummy head is in list, 0 otherwise.
 */
static int list_empty(list_ptr l) {
	return l->prev == l;
}

/**
 * @brief Add an entry after prev
 *
 * @param prev Entry after which the new entry is added.
 * @param entry Entry to be added.
 */
static void list_add(list_ptr prev, list_ptr entry) {
	entry->next = prev->next;
	entry->prev = prev;
	prev->next->prev = entry;
	prev->next = entry;
}

void list_init(list_ptr head) {
	head->prev = head->next = head;
}
 
list_ptr list_remv(list_ptr entry) {
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	return entry;
}

list_ptr list_remv_head(list_ptr l) {
	if (list_empty(l))
    /* After removing the sole item, the list is empty */
		return NULL;
  /* The first data element is the one after the head as the list is empty
   * when there is only 1 element in it. */
	return list_remv(l->next);
}

void list_add_tail(list_ptr l, list_ptr entry) {
	return list_add(l->prev, entry);
}
