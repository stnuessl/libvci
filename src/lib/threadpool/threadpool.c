#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/eventfd.h>

#include "queue.h"
#include "map.h"
#include "task.h"
#include "threadpool.h"
#include "macros.h"

static unsigned int _thread_hash(const void *thread)
{
    size_t size;
    const char *buf;
    unsigned int hval;
    
    buf = (const char *)thread;
    size = sizeof(pthread_t);
    
    hval = 1;
    
    while(size--) {
        hval += *buf++;
        hval += (hval << 10);
        hval ^= (hval >> 6);
        hval &= 0x0fffffff;
    }
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    return hval;
}

static int _thread_compare(const void *a, const void *b)
{
    return !pthread_equal(*(pthread_t *) a, *(pthread_t *) b);
}

static void _thread_delete(void *thread)
{
    pthread_cancel(*(pthread_t *)thread);
    pthread_join(*(pthread_t *)thread);
    free(thread);
}

static void _thread_exit(struct threadpool *__restrict pool)
{
    pthread_t *t, thread;
    
    thread = pthread_self();
    
    pthread_mutex_lock(&pool->mutex_map);

    t = map_take(&pool->thread_map, &thread);
    
    pthread_mutex_unlock(&pool->mutex_map);
    
    free(t);
    
    pthread_exit(NULL);
}

static void *_thread_handle_tasks(void *arg)
{
    struct link *link;
    struct task *task;
    struct threadpool *pool;
    int err;
    
    pool = arg;
    
    while(1) {
        pthread_mutex_lock(&pool->mutex_data);
        
        if(pool->threads_to_exit > 0) {
            pool->threads_to_exit -= 1;
            pthread_mutex_unlock(&pool->mutex_data);
            _thread_exit(pool);
        }
        
        pthread_mutex_unlock(&pool->mutex_data);

        err = sem_wait(&pool->sem_queue_in);
        if(err < 0) {
            if(errno == EINTR)
                continue;
            
            _thread_exit(pool);
        }
        
        pthread_mutex_lock(&pool->mutex_queue_in);
        
        link = queue_take(&pool->task_queue_in);
        task = container_of(link, struct task, link);
        
        pthread_cleanup_push((void(*)(void *)) &task_delete, task);
        
        pthread_mutex_unlock(&pool->mutex_queue_in);

        task->ret_val = task->func(task->arg);
        
        pthread_cleanup_pop(0);

        pthread_mutex_lock(&pool->mutex_queue_out);
        
        err = sem_post(&pool->sem_queue_out);
        if(err < 0)
            task_delete(task);
        else
            queue_insert(&pool->task_queue_out, &task->link);

        pthread_mutex_unlock(&pool->mutex_queue_out);
        
        /* don't know what to do if this fails :-/ */
        eventfd_write(pool->event_fd, 1);
        
        pthread_testcancel();
    }
    
    return NULL;
}

struct threadpool *threadpool_new(int threads)
{
    struct threadpool *pool;
    
    pool = malloc(sizeof(*pool));
    if(!pool)
        return NULL;
    
    if(threadpool_init(pool, threads) < 0) {
        free(pool);
        return NULL;
    }
    
    return pool;
}

void threadpool_delete(struct threadpool *__restrict pool)
{
    threadpool_destroy(pool);
    free(pool);
}

int threadpool_init(struct threadpool *__restrict pool, int threads)
{
    int i, err;
    
    memset(pool, 0, sizeof(*pool));
    
    pool->event_fd = eventfd(0, 0);
    if(pool->event_fd < 0) {
        err = -errno;
        goto out;
    }
    
    err = pthread_mutex_init(&pool->mutex_queue_in, NULL);
    if(err)
        goto cleanup1;
    
    err = pthread_mutex_init(&pool->mutex_queue_out, NULL);
    if(err)
        goto cleanup2;
    
    err = pthread_mutex_init(&pool->mutex_map, NULL);
    if(err)
        goto cleanup3;
    
    err = pthread_mutex_init(&pool->mutex_data, NULL);
    if(err)
        goto cleanup4;

    err = sem_init(&pool->sem_queue_in, 0, 0);
    if(err < 0)
        goto cleanup5;
    
    err = sem_init(&pool->sem_queue_out, 0, 0);
    if(err < 0)
        goto cleanup6;
    
    err = map_init(&pool->thread_map, 0, &_thread_compare, &_thread_hash);
    if(err < 0)
        goto cleanup7;
    
    map_set_key_compare(&pool->thread_map, &_thread_compare);
    map_set_data_delete(&pool->thread_map, &_thread_delete);
    
    for(i = 0; i < threads; ++i) {
        err = threadpool_add_thread(pool);
        if(err < 0)
            goto cleanup8;
    }
    
    queue_init(&pool->task_queue_in);
    queue_init(&pool->task_queue_out);

    return 0;

cleanup8:
    map_destroy(&pool->thread_map);
cleanup7:
    sem_destroy(&pool->sem_queue_out);
cleanup6:
    sem_destroy(&pool->sem_queue_in);
cleanup5:
    pthread_mutex_destroy(&pool->mutex_data);
cleanup4:
    pthread_mutex_destroy(&pool->mutex_map);
cleanup3:
    pthread_mutex_destroy(&pool->mutex_queue_out);
cleanup2:
    pthread_mutex_destroy(&pool->mutex_queue_in);
cleanup1:
    close(pool->event_fd);
out:
    return (err > 0) ? -err : err;
}

void threadpool_destroy(struct threadpool *__restrict pool)
{
    close(pool->event_fd);
    
    map_destroy(&pool->thread_map);
    queue_destroy(&pool->task_queue_out, &task_delete_by_link);
    queue_destroy(&pool->task_queue_in, &task_delete_by_link);
    
    sem_destroy(&pool->sem_queue_out);
    sem_destroy(&pool->sem_queue_in);
    
    pthread_mutex_destroy(&pool->mutex_data);
    pthread_mutex_destroy(&pool->mutex_map);
    pthread_mutex_destroy(&pool->mutex_queue_out);
    pthread_mutex_destroy(&pool->mutex_queue_in);
}

inline int threadpool_event_fd(const struct threadpool *__restrict pool)
{
    return pool->event_fd;
}

int threadpool_add_thread(struct threadpool *__restrict pool)
{
    pthread_t *thread;
    pthread_attr_t attr;
    int err;
    
    thread = malloc(sizeof(*thread));
    if(!thread) {
        err = -errno;
        goto out;
    }

    err = pthread_attr_init(&attr);
    if(err)
        goto cleanup1;
    
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(err)
        goto cleanup2;
    
    err = pthread_create(thread, &attr, &_thread_handle_tasks, pool);
    if(err)
        goto cleanup2;
    
    pthread_mutex_lock(&pool->mutex_map);
    err = map_insert(&pool->thread_map, thread, thread);
    pthread_mutex_unlock(&pool->mutex_map);
    
    if(err < 0)
        goto cleanup3;
    
    pthread_attr_destroy(&attr);
    
    return 0;

cleanup3:
    pthread_cancel(*thread);
cleanup2:
    pthread_attr_destroy(&attr);
cleanup1:
    free(thread);
out:
    return (err > 0) ? -err : err;
}

void threadpool_remove_thread(struct threadpool *__restrict pool)
{
    pthread_mutex_lock(&pool->mutex_data);
    
    pool->threads_to_exit += 1;
    
    pthread_mutex_unlock(&pool->mutex_data);
}

int threadpool_add_task(struct threadpool *__restrict pool, 
                        struct task *task)
{
    int err;

    pthread_mutex_lock(&pool->mutex_queue_in);
    
    err = sem_post(&pool->sem_queue_in);
    if(err < 0) {
        pthread_mutex_unlock(&pool->mutex_queue_in);
        return -errno;
    }
    
    queue_insert(&pool->task_queue_in, &task->link);
    pthread_mutex_unlock(&pool->mutex_queue_in);
    
    pthread_mutex_lock(&pool->mutex_data);
    pool->tasks_inserted += 1;
    pthread_mutex_unlock(&pool->mutex_data);
    
    return err;
}

struct task *threadpool_take_completed_task(struct threadpool *__restrict pool)
{
    struct link *link;
    int err;
    
again:
    err = sem_wait(&pool->sem_queue_out);
    if(err < 0) {
        if(errno == EINTR)
            goto again;
        else
            return NULL;
    }
    
    pthread_mutex_lock(&pool->mutex_queue_out);
    link = queue_take(&pool->task_queue_out);
    pthread_mutex_unlock(&pool->mutex_queue_out);
    
    pthread_mutex_lock(&pool->mutex_data);
    pool->tasks_taken += 1;
    pthread_mutex_unlock(&pool->mutex_data);
    
    return container_of(link, struct task, link);
}

bool threadpool_has_completed_task(struct threadpool *__restrict pool)
{
    bool ret;
    
    pthread_mutex_lock(&pool->mutex_queue_out);
    
    ret = queue_size(&pool->task_queue_out) != 0;
    
    pthread_mutex_unlock(&pool->mutex_queue_out);
    
    return ret;
}

bool threadpool_idle(struct threadpool *__restrict pool)
{
    bool ret;
    
    pthread_mutex_lock(&pool->mutex_data);
    
    ret = pool->tasks_taken == pool->tasks_inserted;
    
    pthread_mutex_unlock(&pool->mutex_data);
    
    return ret;
}

int threadpool_threads_available(struct threadpool *__restrict pool)
{
    int ret;
    
    pthread_mutex_lock(&pool->mutex_data);
    
    ret = map_count(&pool->thread_map);
    
    pthread_mutex_unlock(&pool->mutex_data);
    
    return ret;
}