#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "list.h"
/*
 * Thanks to the wayland devs for this slick list implementation.
 */

inline void list_init(struct list *__restrict list)
{
    list->prev = list;
    list->next = list;
}

inline void list_insert(struct list *list, struct list *item)
{
    item->prev = list;
    item->next = list->next;

    list->next = item;
    item->prev->next = item;
}

inline void list_take(struct list *__restrict item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    
    item->next = NULL;
    item->prev = NULL;
}

inline bool list_empty(const struct list *__restrict list)
{
    return list->next == list;
}

void list_insert_list(struct list *list, struct list *other)
{
    if(list_empty(other))
        return;
    
    other->prev->next = list;
    other->next->prev = list->next;
    
    list->next->prev = other->prev;
    list->next = other->next;
    
    list_init(other);
}