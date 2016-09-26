#ifndef _LIST_H_
#define _LIST_H_
typedef struct list{
	struct list* prev;
	struct list* next;
}list;
void init_list(list *head);
list* list_remv_head(list *l);
void list_add_tail(list *l, list *entry);

/* this is copied from linux 2.6 */
#define LIST_ENTRY(entry_ptr, type, mem) \
		((type *)((char *)(entry_ptr) - (unsigned int)(&((type *)0)->mem)))
#endif
