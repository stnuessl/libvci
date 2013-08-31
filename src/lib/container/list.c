#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


#include "list.h"
#include "item.h"
#include "tree.h"
#include "heap.h"

#define LIST_DEFINE_STATIC_SETGET(name, type)                                  \
                                                                               \
inline void _list_set_##name(struct list *__restrict list, type name)          \
{                                                                              \
    list->_##name = name;                                                      \
}                                                                              \
                                                                               \
inline type _list_get_##name(struct list *__restrict list)                     \
{                                                                              \
    return list->_##name;                                                      \
}                                                                              \

#undef LIST_DEFINE_STATIC_SETGETT

inline static void _list_adjust_size(struct list *__restrict list, int adj)
{
    list->_size += adj;
}

static void _list_insert_item(struct list *__restrict list,
                              struct item *prev, 
                              struct item *next, 
                              struct item *item)
{
    if(prev)
        item_set_next(prev, item);
    else
        list->_head = item;
    
    if(next)
        item_set_prev(next, item);
    else
        list->_tail = item;
    
    item_set_prev(item, prev);
    item_set_next(item, next);
    
    _list_adjust_size(list, 1);
}

static int _list_insert(struct list *__restrict list, 
                        void *__restrict data,
                        void *__restrict key,
                        void (*insert)(struct list *__restrict list, 
                                       struct item *__restrict item))
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    insert(list, item);
    
    return 0;
}

static void *_list_take(struct list *__restrict list,
                        struct item *(*take)(struct list *__restrict),
                        void (*key_delete)(void *))
{
    struct item *item;
    void *data;
    
    item = take(list);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, key_delete);
    
    return data;
}

static struct item *_list_take_item(struct list *__restrict list, 
                                    struct item *item)
{
    struct item *head;
    struct item *tail;
    struct item *prev;
    struct item *next;
    
    head = list->_head;
    tail = list->_tail;
    prev = item_prev(item);
    next = item_next(item);
    
    if(prev)
        item_set_next(prev, next);
    
    if(next)
        item_set_prev(next, prev);

    if(head == item)
        list->_head = item_next(head);
    
    if(tail == item)
        list->_tail = item_prev(tail);
    
    item_set_prev(item, NULL);
    item_set_next(item, NULL);
    
    _list_adjust_size(list, -1);
    
    return item;
}

struct list *list_new(void)
{
    struct list *list;
    
    list = malloc(sizeof(*list));
    if(!list)
        return NULL;
    
    list_init(list);
    
    return list;
}

void list_delete(struct list *__restrict list, 
                 void (*data_delete)(void *),
                 void (*key_delete)(void *))
{
    list_destroy(list, data_delete, key_delete);
    free(list);
}

void list_init(struct list *__restrict list)
{
    memset(list, 0, sizeof(*list));
}

void list_destroy(struct list *__restrict list,
                  void (*data_delete)(void *),
                  void (*key_delete)(void *))
{
    struct item *item;
    
    while(!list_empty(list)) {
        item = list_take_item_front(list);
        
        item_delete(item, data_delete, key_delete);
    }
}


int list_insert_front(struct list *__restrict list, 
                      void *__restrict data, 
                      void *__restrict key)
{
    return _list_insert(list, data, key, &list_insert_item_front);
}

int list_insert_back(struct list *__restrict list, 
                     void *__restrict data, 
                     void *__restrict key)
{
    return _list_insert(list, data, key, &list_insert_item_back);
}

void list_insert_item_front(struct list *__restrict list, 
                       struct item *__restrict item)
{
    _list_insert_item(list, NULL, list->_head, item);
}

void list_insert_item_back(struct list *__restrict list, 
                      struct item *__restrict item)
{
    _list_insert_item(list, list->_tail, NULL, item);
}

void *list_take_front(struct list *__restrict list, void (*key_delete)(void *))
{
    return _list_take(list, &list_take_item_front, key_delete);
}

void *list_take_back(struct list *__restrict list, void (*key_delete)(void *))
{
    return _list_take(list, &list_take_item_back, key_delete);
}

struct item *list_take_item_front(struct list *__restrict list)
{
    return _list_take_item(list, list->_head);;
}

struct item *list_take_item_back(struct list *__restrict list)
{
    return _list_take_item(list, list->_tail);;
}

void *list_take(struct list *__restrict list, 
                void *__restrict key, 
                int (*key_compare)(const void *, const void *), 
                void (*key_delete)(void *))
{
    struct item *item;
    void *data;
    
    item = list_take_item(list, key, key_compare);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, key_delete);
    
    return data;
}

struct item *list_take_item(struct list *__restrict list, 
                       const void *__restrict key, 
                       int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    item = list_retrieve_item(list, key, key_compare);
    if(!item)
        return NULL;
    
    return _list_take_item(list, item);;
}

void *list_retrieve(struct list *__restrict list, 
                    const void *__restrict key,
                    int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    item = list_retrieve_item(list, key, key_compare);
    
    return (item) ? item_data(item) : NULL;
}

struct item *list_retrieve_item(struct list *__restrict list, 
                                const void *__restrict key,
                                int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    list_for_each(list, item) {
        if(!key_compare(item_key(item), key))
            return item;
    }

    return NULL;
}

void list_delete_item(struct list *__restrict list,
                      const void *__restrict key,
                      int (*key_compare)(const void *, const void *),
                      void (*data_delete)(void *),
                      void (*key_delete)(void *))
{
    struct item *item;
    
    item = list_take_item(list, key, key_compare);
    
    item_delete(item, data_delete, key_delete);
}

struct item *list_begin(struct list *__restrict list)
{
    return list->_head;
}

struct item *list_end(struct list *__restrict list)
{
    return list->_tail;
}

inline int list_size(const struct list *__restrict list)
{
    return list->_size;
}

inline bool list_empty(const struct list *__restrict list)
{
    return list_size(list) == 0;
}

void list_sort(struct list *__restrict list, 
               int (*key_compare)(const void *, const void *))
{
    struct tree heap;
    
    tree_init(&heap);
    
    tree_set_key_compare(&heap, key_compare);
    
    while(!list_empty(list))
        heap_insert_item(&heap, list_take_item_front(list));
    
    *list = heap_sort(&heap);
    
    tree_destroy(&heap);
}

int sorted_list_insert(struct list *__restrict list, 
                       void *__restrict data, 
                       void *__restrict key,
                       int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    sorted_list_insert_item(list, item, key_compare);
    
    return 0;
}

void sorted_list_insert_item(struct list *__restrict list,
                             struct item *__restrict item,
                             int (*key_compare)(const void *, const void *))
{
    struct item *prev, *next;
    
    next = list->_head;
    
    while(next) {
        if(key_compare(item_key(item), item_key(next)) > 0)
            next = item_next(next);
        else
            break;
    }
    
    prev = (next) ? item_prev(next) : list->_tail;
    
    _list_insert_item(list, prev, next, item);
}

void *sorted_list_retrieve(struct list *__restrict list,
                           const void * __restrict key,
                           int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    item = sorted_list_retrieve_item(list, key, key_compare);
    
    return (item) ? item_data(item) : NULL;
}

struct item *sorted_list_retrieve_item(struct list *__restrict list,
                                       const void *__restrict key,
                                int (*key_compare)(const void *, const void *))
{
    struct item *head, *tail, *item;
    int size, res;
    
    head = list->_head;
    tail = list->_tail;
    size = list->_size;
    
    while(1) {
        size -= (size == 1) ? 1 : (size >> 1);
        
        item = item_nth_next(head, size);
        
        res = key_compare(key, item_key(item));
        
        if(res > 0)
            head = item;
        else if(res < 0)
            tail = item;
        else
            return item;
        
        if(head == tail)
            break;
    }
    
    return NULL;
}

void *sorted_list_take(struct list *__restrict list, 
                       void *__restrict key,
                       int(*key_compare)(const void *, const void *),
                       void (*key_delete)(void *))
{
    struct item *item;
    void *data;
    
    item = sorted_list_take_item(list, key, key_compare);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, key_delete);
    
    return data;
}

struct item *sorted_list_take_item(struct list *__restrict list, 
                                   void *__restrict key,                        
                                int (*key_compare)(const void *, const void *))
{
    struct item *item;
    
    item = sorted_list_retrieve_item(list, key, key_compare);
    if(!item)
       return NULL;
    
    return _list_take_item(list, item);
}


#define LIST_DEFINE_SETGET_CALLBACK(name, ret_type, param)                     \
                                                                               \
inline void list_set_##name(struct list *__restrict list,                      \
                            ret_type (*name)param)                             \
{                                                                              \
    list->_##name = name;                                                      \
}                                                                              \
                                                                               \
inline ret_type (*list_get_##name(struct list *__restrict))param               \
{                                                                              \
    return list->_##name;                                                      \
}                                                                              \

#undef LIST_DEFINE_SET_CALLBACK