#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "map.h"
#include "queue.h"
#include "task.h"

struct threadpool {
    int event_fd;
    
    struct queue task_queue_in;
    struct queue task_queue_out;
    struct map thread_map;
    
    unsigned int threads_to_exit;
    unsigned int tasks_inserted;
    unsigned int tasks_completed;
    unsigned int tasks_taken;
    
    sem_t sem_queue_in;
    sem_t sem_queue_out;
    
    pthread_mutex_t mutex_queue_in;
    pthread_mutex_t mutex_queue_out;
    pthread_mutex_t mutex_map;
    pthread_mutex_t mutex_data;
};

struct threadpool *threadpool_new(int threads);

void threadpool_delete(struct threadpool *__restrict pool);

int threadpool_init(struct threadpool *__restrict pool, int threads);

void threadpool_destroy(struct threadpool *__restrict pool);

inline int threadpool_event_fd(const struct threadpool *__restrict pool);

int threadpool_add_thread(struct threadpool *__restrict pool);

void threadpool_remove_thread(struct threadpool *__restrict pool);

int threadpool_add_task(struct threadpool *__restrict pool, 
                        struct task *task);

struct task *threadpool_take_completed_task(struct threadpool *__restrict pool);

bool threadpool_has_completed_task(struct threadpool *__restrict pool);

bool threadpool_idle(struct threadpool *__restrict pool);

int threadpool_threads_available(struct threadpool *__restrict pool);


#endif /* _THREADPOOL_H_ */