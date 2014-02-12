#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdbool.h>

struct heap {
    void **data;
    
    void (*data_delete)(void *);
    int (*data_compare)(const void *, const void *);
    
    unsigned int size;
    unsigned int capacity;
};


struct heap *heap_new(unsigned int capacity, 
                      int (*data_compare)(const void *, const void *));

void heap_delete(struct heap *__restrict heap);

int heap_init(struct heap *__restrict heap, 
              unsigned int capacity, 
              int (*data_compare)(const void *, const void *));

void heap_destroy(struct heap *__restrict heap);

void heap_clear(struct heap *__restrict heap);

int heap_insert(struct heap *__restrict heap, void *data);

void *heap_take(struct heap *__restrict heap);

void *heap_retrieve(struct heap *__restrict heap);

bool heap_empty(struct heap *__restrict heap);

unsigned int heap_size(struct heap *__restrict heap);

void heap_set_data_delete(struct heap *__restrict heap,
                          void (*data_delete)(void *));

void (*heap_data_delete(struct heap *__restrict heap))(void *);

void heap_set_data_compare(struct heap *__restrict heap,
                           int (*data_compare)(const void *, const void*));

int (*heap_data_compare(struct heap *__restrict heap))
                       (const void *, const void *);

#endif /* _HEAP_H_ */