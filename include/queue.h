#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>

#include "item.h"
#include "list.h"

struct queue {
    struct list _list;
    
    void (*_data_delete)(void *);
    void (*_key_delete)(void *);
};

struct queue *queue_new(void);

void queue_delete(struct queue *__restrict queue);

void queue_init(struct queue *__restrict queue);

void queue_destroy(struct queue *__restrict queue);

int queue_enqueue(struct queue *__restrict queue, 
                  void *__restrict data, 
                  void *__restrict key);

void queue_enqeue_item(struct queue *__restrict queue, struct item *item);

void *queue_dequeue(struct queue *__restrict queue);

struct item *queue_dequeue_item(struct queue *__restrict queue);

void *queue_head(struct queue *__restrict queue);

struct item *queue_head_item(struct queue *__restrict queue);

void queue_delete_head(struct queue *__restrict queue);

int queue_size(const struct queue *__restrict queue);

bool queue_empty(const struct queue *__restrict queue);

#define QUEUE_DEFINE_SET_CALLBACK(name, type, param)                           \
                                                                               \
inline void queue_set_##name(struct queue *__restrict queue,                   \
                             type (*name)param);

QUEUE_DEFINE_SET_CALLBACK(key_delete, void, (void *))
QUEUE_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef QUEUE_DEFINE_SET_CALLBACK


#endif /* _QUEUE_H_ */