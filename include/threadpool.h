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

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "link.h"
#include "map.h"
#include "queue.h"

struct threadpool_task {
    struct link link;
    void (*func)(struct threadpool_task *);
};

struct threadpool {
    struct queue task_queue_in;
    struct queue task_queue_out;
    struct map thread_map;

    sem_t sem_queue_in;
    sem_t sem_queue_out;
    sem_t sem_exit;
    
    pthread_mutex_t mutex_queue_in;
    pthread_mutex_t mutex_queue_out;
    pthread_mutex_t mutex_map;
    
    int event_fd;
};

struct threadpool *threadpool_new(int threads);

void threadpool_delete(struct threadpool *__restrict pool);

int threadpool_init(struct threadpool *__restrict pool, int threads);

void threadpool_destroy(struct threadpool *__restrict pool);

int threadpool_event_fd(const struct threadpool *__restrict pool);

int threadpool_add_thread(struct threadpool *__restrict pool);

int threadpool_remove_thread(struct threadpool *__restrict pool);

int threadpool_add_task(struct threadpool *__restrict pool, 
                        struct threadpool_task *task);

struct threadpool_task *
threadpool_take_completed_task(struct threadpool *__restrict pool);

unsigned int threadpool_tasks_queued(struct threadpool *pool);

#endif /* _THREADPOOL_H_ */