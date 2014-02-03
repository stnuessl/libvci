#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#include "link.h"
#include "list.h"
/*
 * Thanks to the wayland devs for this slick list implementation.
 */

inline void list_init(struct link *__restrict list)
{
    list->prev = list;
    list->next = list;
}

void list_destroy(struct link *__restrict list,
                  void (*data_delete)(struct link *))
{
    list_clear(list, data_delete);
}

void list_clear(struct link *__restrict list,
                void (*data_delete)(struct link *))
{
    struct link *link, *next;
    
    if(!data_delete) {
        list_init(list);
        return;
    }
    
    list_for_each_safe(list, link, next)
        data_delete(link);
}

inline void list_insert(struct link *list, struct link *link)
{
    link->prev = list;
    link->next = list->next;

    list->next = link;
    link->next->prev = link;
}

inline void list_take(struct link *__restrict link)
{
    link->prev->next = link->next;
    link->next->prev = link->prev;
}

inline void list_insert_front(struct link *__restrict list, struct link *link)
{
    list_insert(list, link);
}

inline void list_insert_back(struct link *__restrict list, struct link *link)
{
    list_insert(list->prev, link);
}

inline struct link *list_front(struct link *__restrict list)
{
    return list->next;
}

inline struct link *list_back(struct link *__restrict list)
{
    return list->prev;
}

struct link *list_take_front(struct link *__restrict list)
{
    struct link *link;
    
    link = list_front(list);
    
    list_take(link);
    
    return link;
}

struct link *list_take_back(struct link *__restrict list)
{
    struct link *link;
    
    link = list_back(list);
    
    list_take(link);
    
    return link;
}

inline bool list_empty(const struct link *__restrict list)
{
    return list->next == list;
}

void list_insert_list(struct link *list, struct link *other)
{
    if(list_empty(other))
        return;
    
    other->prev->next = list;
    other->next->prev = list->next;
    
    list->next->prev = other->prev;
    list->next = other->next;
    
    list_init(other);
}

void list_assert(const struct link *__restrict list)
{
    const struct link *link;
    
    list_for_each(list, link) {
        assert(link->prev->next == link && "Invalid successor of prev. link.");
        assert(link->next->prev == link && "Invalid predecessor of next link.");
    }
}