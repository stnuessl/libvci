
#ifndef _LIST_H_
#define _LIST_H_

#include "item.h"

#include <stdbool.h>

struct list {
    struct item *_head;
    struct item *_tail;
    
    int _size;
};

struct list *list_new(void);

void list_delete(struct list *__restrict list, 
                 void (*data_delete)(void *),
                 void (*key_delete)(void *));

void list_init(struct list *__restrict list);

void list_destroy(struct list *__restrict list,
                  void (*data_delete)(void *),
                  void (*key_delete)(void *));

int list_insert_front(struct list *__restrict list, 
                      void *__restrict data, 
                      void *__restrict key);

int list_insert_back(struct list *__restrict list, 
                     void *__restrict data, 
                     void *__restrict key);

void list_insert_item_front(struct list *__restrict list, 
                            struct item *__restrict item);

void list_insert_item_back(struct list *__restrict list, 
                           struct item *__restrict item);

void *list_take_front(struct list *__restrict list, void (*key_delete)(void *));

void *list_take_back(struct list *__restrict list, void (*key_delete)(void *));

struct item *list_take_item_front(struct list *__restrict list);

struct item *list_take_item_back(struct list *__restrict list);

void *list_take(struct list *__restrict list, 
                void *__restrict key, 
                int (*key_compare)(const void *, const void *), 
                void (*key_delete)(void *));

struct item *list_take_item(struct list *__restrict list, 
                       const void *__restrict key, 
                       int (*key_compare)(const void *, const void *));

void *list_retrieve(struct list *__restrict, 
                    const void *__restrict key,
                    int (*key_compare)(const void *, const void *));

struct item *list_retrieve_item(struct list *__restrict list, 
                                const void *__restrict key,
                                int (*key_compare)(const void *, const void *));

void list_delete_item(struct list *__restrict list,
                      const void *__restrict key,
                      int (*key_compare)(const void *, const void *),
                      void (*data_delete)(void *),
                      void (*key_delete)(void *));

struct item *list_begin(struct list *__restrict list);

struct item *list_end(struct list *__restrict list);

inline int list_size(const struct list *__restrict list);

inline bool list_empty(const struct list *__restrict list);

void list_sort(struct list *__restrict list, 
               int (*key_compare)(const void *, const void *));

int sorted_list_insert(struct list *__restrict list, 
                       void *__restrict data, 
                       void *__restrict key,
                       int (*key_compare)(const void *, const void *));

void sorted_list_insert_item(struct list *__restrict list,
                             struct item *__restrict item,
                             int (*key_compare)(const void *, const void *));

void *sorted_list_retrieve(struct list *__restrict list,
                              const void * __restrict key,
                              int (*key_compare)(const void *, const void *));

struct item *sorted_list_retrieve_item(struct list *__restrict list,
                                     const void *__restrict key,
                                int (*key_compare)(const void *, const void *));

void *sorted_list_take(struct list *__restrict list, 
                       void *__restrict key,
                       int(*key_compare)(const void *, const void *),
                       void (*key_delete)(void *));

struct item *sorted_list_take_item(struct list *__restrict list, 
                                   void *__restrict key,                        
                                int (*key_compare)(const void *, const void *));
#define list_for_each(list, item)                                              \
for((item) = list_begin((list)); (item); (item) = item_next((item)))

#endif /* _LIST_H_ */