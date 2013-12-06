
#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>

struct list {
    struct list *prev;
    struct list *next;
};


inline void list_init(struct list *__restrict list);

inline void list_insert(struct list *list, struct list *item);

inline void list_take(struct list *__restrict item);

inline bool list_empty(const struct list *__restrict list);

void list_insert_list(struct list *list, struct list *other);

#define list_for_each(list, item)                                              \
for((item) = (list)->next; (item) != (list); (item) = (item)->next)
    
#define list_for_each_safe(list, item, tmp)                                    \
    for((item) = (list)->next, (tmp) = (item)->next;                           \
        (item) != (list);                                                      \
        (item) = (tmp), (tmp) = (tmp)->next)
        
#define list_for_each_reverse(list, item)                                      \
        for((item) = (list)->prev; (item) != (list); (item) = (item)->prev)
            
#define list_for_each_reverse_safe(list, item, tmp)                            \
    for((item) = (list)->prev; (tmp) = (item)->prev;                           \
        (item) != (list);                                                      \
        (item) = (tmp), (tmp) = (tmp)->prev)
                
#endif /* _LIST_H_ */