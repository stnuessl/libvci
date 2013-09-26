

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include <map.h>
#include <queue.h>

#include "task.h"

struct threadpool {
    struct queue _queue_in;
    struct queue _queue_out;
    struct map _map_threads;
    
    int _threads_to_exit;
    int _tasks_inserted;
    int _tasks_taken;
    
    sem_t _sem_in;
    sem_t _sem_out;
    
    pthread_mutex_t _mutex_in;
    pthread_mutex_t _mutex_out;
    pthread_mutex_t _mutex_data;
};

struct threadpool *threadpool_new(int threads);

void threadpool_delete(struct threadpool *__restrict pool);

int threadpool_init(struct threadpool *__restrict pool, int threads);

int threadpool_add_thread(struct threadpool *__restrict pool);

void threadpool_remove_thread(struct threadpool *__restrict pool);

void threadpool_destroy(struct threadpool *__restrict pool);

int threadpool_add_task(struct threadpool *__restrict pool, 
                        struct task *task);

struct task *threadpool_take_completed_task(struct threadpool *__restrict pool);

bool threadpool_has_completed_task(struct threadpool *__restrict pool);

bool threadpool_done(struct threadpool *__restrict pool);

int threadpool_threads_available(struct threadpool *__restrict pool);


#endif /* _THREADPOOL_H_ */