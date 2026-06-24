#ifndef __LIST_H__
#define __LIST_H__ 1
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
struct List{
	struct List *prev;
	struct List *next;
};
static inline __always_inline void list_init(struct List* list){
	list->prev=list;
	list->next=list;
}
static inline __always_inline void list_add_to_behind(struct List* list,struct List* entry){
	entry->next=list->next;
	entry->prev=list;
	entry->next->prev=entry;
	list->next=entry;
}
static inline __always_inline void list_add_to_before(struct List* list,struct List* entry){
	entry->next=list;
	entry->prev=list->prev;
	entry->prev->next=entry;
	list->prev=entry;
}
static inline __always_inline void list_del(struct List *entry){
	entry->next->prev=entry->prev;
	entry->prev->next=entry->next;
}
static inline __always_inline uint64_t list_is_empty(struct List *entry){
	if(entry==entry->next&&entry->prev==entry)	return 1;
	else    return 0;
}
static inline __always_inline struct List* list_prev(struct List* list){return list->prev;}
static inline __always_inline struct List* list_next(struct List* list){return list->next;}
#endif
