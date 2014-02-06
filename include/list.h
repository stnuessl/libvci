#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>

#include "link.h"

struct link *list_new(void);

void list_delete(struct link *__restrict list,
                  void (*data_delete)(struct link *));

void list_init(struct link *__restrict list);

void list_destroy(struct link *__restrict list,
                  void (*data_delete)(struct link *));

void list_clear(struct link *__restrict list,
                void (*data_delete)(struct link *));

void list_insert(struct link *list, struct link *link);

void list_take(struct link *__restrict link);

void list_insert_front(struct link *__restrict list, struct link *link);

void list_insert_back(struct link *__restrict list, struct link *link);

struct link *list_front(struct link *__restrict list);

struct link *list_back(struct link *__restrict list);

struct link *list_take_front(struct link *__restrict list);

struct link *list_take_back(struct link *__restrict list);

bool list_empty(const struct link *__restrict list);

void list_insert_list(struct link *list, struct link *other);

#define list_for_each(list, link)                                              \
    for((link) = (list)->next; (link) != (list); (link) = (link)->next)

#define list_for_each_reverse(list, link)                                      \
    for((link) = (list)->prev; (link) != (list); (link) = (link)->prev)

#define list_for_each_safe(list, link, safe)                                   \
    for((link) = (list)->next, (safe) = (link)->next;                          \
        (link) != (list);                                                      \
        (link) = (safe), (safe) = (safe)->next)

#define list_for_each_reverse_safe(list, link, safe)                           \
    for((link) = (list)->prev, (safe) = (link)->prev;                          \
        (link) != (list);                                                      \
        (link) = (safe), (safe) = (safe)->prev)

#endif /* _LIST_H_ */