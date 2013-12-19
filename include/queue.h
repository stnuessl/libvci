#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>

#include "link.h"

struct queue {
    struct link list;
    
    unsigned int size;
};

struct queue *queue_new(void);

void queue_delete(struct queue *__restrict queue, 
                  void (*data_delete)(struct link *));

void queue_init(struct queue *__restrict queue);

void queue_destroy(struct queue *__restrict queue,
                   void (*data_delete)(struct link *));

void queue_clear(struct queue *__restrict queue,
                 void (*data_delete)(struct link *));

void queue_insert(struct queue *__restrict queue, struct link *link);

struct link *queue_take(struct queue *__restrict queue);

int queue_size(const struct queue *__restrict queue);

bool queue_empty(const struct queue *__restrict queue);

#endif /* _QUEUE_H_ */