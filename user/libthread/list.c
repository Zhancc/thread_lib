#include <list.h>

static int list_empty(list *l){
	return l->prev == l->next;
}
void init_list(list *head){
	head->prev = head->next = head;
}
 
static list* list_remv(list *entry){
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	return entry;
}

static void list_add(list *entry, list *prev){
	entry->next = prev->next;
	entry->prev = prev;
	prev->next = entry;
}
list* list_remv_head(list *l){
	if(list_empty(l))
		return 0;
	return list_remv(l->next);
}

void list_add_tail(list *l, list *entry){
	return list_add(entry, l->prev->prev);
}
