#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "link.h"
#include "list.h"
#include "queue.h"


struct queue *queue_new(void)
{
    struct queue *queue;
    
    queue = malloc(sizeof(*queue));
    if(!queue)
        return NULL;
    
    queue_init(queue);
    
    return queue;
}

void queue_delete(struct queue *__restrict queue, 
                  void (*data_delete)(struct link *))
{
    queue_destroy(queue, data_delete);
    free(queue);
}

void queue_init(struct queue *__restrict queue)
{
    list_init(&queue->list);
    
    queue->size = 0;
}

void queue_destroy(struct queue *__restrict queue,
                   void (*data_delete)(struct link *))
{
    list_destroy(&queue->list, data_delete);
}

void queue_clear(struct queue *__restrict queue,
                 void (*data_delete)(struct link *))
{
    queue->size = 0;
    
    list_clear(&queue->list, data_delete);
}

void queue_insert(struct queue *__restrict queue, struct link *link)
{
    queue->size += 1;
    
    list_insert_front(&queue->list, link);
}

struct link *queue_take(struct queue *__restrict queue)
{
    queue->size -= 1;
    
    return list_take_back(&queue->list);
}

int queue_size(const struct queue *__restrict queue)
{
    return queue->size;
}

bool queue_empty(const struct queue *__restrict queue)
{
    return queue->size == 0;
}