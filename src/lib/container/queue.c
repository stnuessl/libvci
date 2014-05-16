/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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