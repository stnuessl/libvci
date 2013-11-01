#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "item.h"
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

void queue_delete(struct queue *__restrict queue)
{
    queue_destroy(queue);
    free(queue);
}

void queue_init(struct queue *__restrict queue)
{
    memset(queue, 0, sizeof(*queue));
    list_init(&queue->_list);
}

void queue_destroy(struct queue *__restrict queue)
{
    list_destroy(&queue->_list, queue->_data_delete, queue->_key_delete);
}

int queue_enqueue(struct queue *__restrict queue, 
                  void *__restrict data, 
                  void *__restrict key)
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    queue_enqeue_item(queue, item);
    
    return 0;
}


void queue_enqeue_item(struct queue *__restrict queue, struct item *item)
{
    list_insert_item_back(&queue->_list, item);
}

void *queue_dequeue(struct queue *__restrict queue)
{
    struct item *item;
    void *data;
    
    item = queue_dequeue_item(queue);
    
    data = item_data(item);
    
    item_delete(item, NULL, queue->_key_delete);
    
    return data;
}

struct item *queue_dequeue_item(struct queue *__restrict queue)
{
    return list_take_item_front(&queue->_list);
}

void *queue_head(struct queue *__restrict queue)
{
    return item_data(queue_head(queue));
}

struct item *queue_head_item(struct queue *__restrict queue)
{
    return list_begin(&queue->_list);
}

void queue_delete_head(struct queue *__restrict queue)
{
    struct item *item;
    
    item = queue_dequeue_item(queue);
    
    item_delete(item, queue->_data_delete, queue->_key_delete);
}

int queue_size(const struct queue *__restrict queue)
{
    return list_size(&queue->_list);
}

bool queue_empty(const struct queue *__restrict queue)
{
    return list_empty(&queue->_list);
}


#define QUEUE_DEFINE_SET_CALLBACK(name, type, param)                           \
                                                                               \
inline void queue_set_##name(struct queue *__restrict queue,                   \
                             type (*name)param)                                \
{                                                                              \
    queue->_##name = name;                                                     \
}

QUEUE_DEFINE_SET_CALLBACK(key_delete, void, (void *))
QUEUE_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef QUEUE_DEFINE_SET_CALLBACK