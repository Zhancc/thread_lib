/**
 * @file list.h
 * @brief Defines doubly linked list API.
 * 
 * List entry is used as trail guide to find the bigger data structure that 
 * encapsulates it. For example, consider the following declaration.
 *
 *    struct data_point {
 *      int a;
 *      float b;
 *      list entry;
 *      ...(other member variables)
 *    }
 *
 *       -----------
 *       |    a    |
 *       -----------
 *       |    b    |
 *       -----------
 *    <--+----+    |
 *       |  entry  |
 *       |    +----+---> ...
 *       -----------
 *       |         |
 *       |         |
 *
 * Instead of have a list entry that encapulates a pointer to the data_point, we
 * have data_pint encapsulate the list entry. When a data_point is created, its 
 * entry is inserted into the list. As long as we have the address of the entry,
 * we can always find the address of the fisrt element of data_point as it is a 
 * fixed offset away. 
 *
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#ifndef _LIST_H_
#define _LIST_H_

/* TODO Per syllabus, we can't just lift code out of reference code base.
 * need to rewrite this. This is copied from linux 2.6 */
/* 
 * Macros to obtain the pointer to the parent struct, given the pointer to a 
 * member of the parent struct. It does so by finding out the offset byte of 
 * the member variable and subtract that from the member pointer to obtain the 
 * address of the parent data structure.
 *
 * e.g.
 *
 * struct child {
 *   int x;
 * }
 *
 * struct parent {
 *   int a;
 *   int b;
 *   struct child mike;
 * }
 *
 *  parent_ptr         entry_ptr
 *  |                  |
 *  |                  |
 *  V                  V  
 *  |<--   offset  -->|
 *  -------------------------------
 *  |    a   |    b   |    mike   |
 *  -------------------------------
 *
 *
 */
#define OFFSETOF(type, member)  ((unsigned int)(&((type *)0)->member))
#define LIST_ENTRY(entry_ptr, type, member) \
		((entry_ptr) ? \
        ((type *)((char *)(entry_ptr) - OFFSETOF(type, member))): \
        0)

typedef struct _list *list_ptr;
typedef struct _list {
	list_ptr prev;
	list_ptr next;
} list_t;

/**
 * @brief Initialize list structure. Both prev and next points to itself.
 *
 * The list is empty when there is only 1 dummy entry in it.
 *
 * @param head Pointer to dummy head of the list.
 */
void list_init(list_ptr head);

/**
 * @brief Removes an entry from the list and links up the prev and next item.
 *
 * @param entry Pointer to entry to be removed.
 *
 * @return Pointer to entry just removed.
 */
list_ptr list_remv(list_ptr entry);

/**
 * @brief Removes the head entry of the list.
 *
 * @param l Pointer to dummy head of the list.
 *
 * @return Pointer to the severed head.
 */
list_ptr list_remv_head(list_ptr l);

/**
 * @brief Adds a new tail node to the list.
 *
 * @param l Pointer to dummy head of the list.
 * @param entry Pointer to new tail entry.
 */
void list_add_tail(list_ptr l, list_ptr entry);

/**
 * @brief check if the list is empty
 * @param l pointert to the header of the list
 * @return if empty, return 0
 */
int list_empty(list_ptr l);
#endif /* _LIST_H_ */
