#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <item.h>
#include <queue.h>

#include "task.h"
#include "threadpool.h"

static void _task_item_delete(struct item *__restrict item)
{
    item_delete(item, (void(*)(void *))&task_delete, NULL);
}

static int _thread_compare(const void *a, const void *b)
{
    return !pthread_equal(*(pthread_t *) a, *(pthread_t *) b);
}

static void _thread_delete(void *thread)
{
    pthread_cancel(*(pthread_t *)thread);
    free(thread);
}

static void _threadpool_thread_prepare_exit(struct threadpool *__restrict pool)
{
    pthread_t thread;
    
    thread = pthread_self();
    
    /* '_mutex_data' must be locked! */
    free(hash_take(&pool->_hash_threads, &thread));
}

static void *_threadpool_task_handler(void *arg)
{
    struct item *item;
    struct task *task;
    struct threadpool *pool;
    int err;
    
    pool = arg;
    
    while(1) {
        pthread_mutex_lock(&pool->_mutex_data);
        
        if(pool->_threads_to_exit > 0) {
            pool->_threads_to_exit -= 1;
            _threadpool_thread_prepare_exit(pool);
            pthread_mutex_unlock(&pool->_mutex_data);
            pthread_exit(NULL);
        }
        
        pthread_mutex_unlock(&pool->_mutex_data);

        err = sem_wait(&pool->_sem_in);
        if(err < 0) {
            if(errno == EINTR)
                continue;
            
            pthread_mutex_lock(&pool->_mutex_data);
            _threadpool_thread_prepare_exit(pool);
            pthread_mutex_lock(&pool->_mutex_data);
            pthread_exit(NULL);
        }
        
        pthread_mutex_lock(&pool->_mutex_in);
        
        item = queue_dequeue_item(&pool->_queue_in);
        pthread_cleanup_push((void(*)(void *)) &_task_item_delete, item);
        
        pthread_mutex_unlock(&pool->_mutex_in);
        
        task = item_data(item);
        
        /* execute task */
        task->_return_value = task->_func(task->_arg);
        

        pthread_mutex_lock(&pool->_mutex_out);
        
        pthread_cleanup_pop(0);
        
        err = sem_post(&pool->_sem_out);
        if(err < 0)
            _task_item_delete(item);
        else
            queue_enqeue_item(&pool->_queue_out, item);

        pthread_mutex_unlock(&pool->_mutex_out);
        
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
    
    err = pthread_mutex_init(&pool->_mutex_in, NULL);
    if(err)
        goto out;
    
    err = pthread_mutex_init(&pool->_mutex_out, NULL);
    if(err)
        goto cleanup1;
    
    err = pthread_mutex_init(&pool->_mutex_data, NULL);
    if(err)
        goto cleanup2;

    err = sem_init(&pool->_sem_in, 0, 0);
    if(err < 0)
        goto cleanup3;
    
    err = sem_init(&pool->_sem_out, 0, 0);
    if(err < 0)
        goto cleanup4;
    
    err = hash_init(&pool->_hash_threads, 0, sizeof(pthread_t));
    if(err < 0)
        goto cleanup5;
    
    hash_set_key_compare(&pool->_hash_threads, &_thread_compare);
    hash_set_data_delete(&pool->_hash_threads, &_thread_delete);
    
    for(i = 0; i < threads; ++i) {
        err = threadpool_add_thread(pool);
        if(err < 0)
            goto cleanup6;
    }
    
    queue_init(&pool->_queue_in);
    queue_init(&pool->_queue_out);
    
    queue_set_data_delete(&pool->_queue_in, (void(*)(void *)) &task_delete);
    queue_set_data_delete(&pool->_queue_out, (void(*)(void *)) &task_delete);
    
    return 0;

cleanup6:
    hash_destroy(&pool->_hash_threads);
cleanup5:
    sem_destroy(&pool->_sem_out);
cleanup4:
    sem_destroy(&pool->_sem_in);
cleanup3:
    pthread_mutex_destroy(&pool->_mutex_data);
cleanup2:
    pthread_mutex_destroy(&pool->_mutex_out);
cleanup1:
    pthread_mutex_destroy(&pool->_mutex_in);
out:
    return (err > 0) ? -err : err;
}

void threadpool_destroy(struct threadpool *__restrict pool)
{
    
    hash_destroy(&pool->_hash_threads);
    queue_destroy(&pool->_queue_out);
    queue_destroy(&pool->_queue_in);
    
    sem_destroy(&pool->_sem_out);
    sem_destroy(&pool->_sem_in);
    
    pthread_mutex_destroy(&pool->_mutex_data);
    pthread_mutex_destroy(&pool->_mutex_out);
    pthread_mutex_destroy(&pool->_mutex_in);
}

int threadpool_add_thread(struct threadpool *__restrict pool)
{
    struct item *item;
    pthread_t *thread;
    pthread_attr_t attr;
    int err;
    
    thread = malloc(sizeof(*thread));
    if(!thread) {
        err = -errno;
        goto out;
    }
    
    item = item_new(thread, thread);
    if(!item) {
        err = -errno;
        goto cleanup1;
    }
    
    err = pthread_attr_init(&attr);
    if(err)
        goto cleanup2;
    
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(err)
        goto cleanup3;
    
    err = pthread_create(thread, &attr, &_threadpool_task_handler, pool);
    if(err)
        goto cleanup3;
    
    pthread_mutex_lock(&pool->_mutex_data);
    
    hash_insert_item(&pool->_hash_threads, item);
    
    pthread_mutex_unlock(&pool->_mutex_data);
    
    pthread_attr_destroy(&attr);
    
    return 0;

cleanup3:
    pthread_attr_destroy(&attr);
cleanup2:
    item_delete(item, NULL, NULL);
cleanup1:
    free(thread);
out:
    return (err > 0) ? -err : err;
}

void threadpool_remove_thread(struct threadpool *__restrict pool)
{
    pthread_mutex_lock(&pool->_mutex_data);
    
    pool->_threads_to_exit += 1;
    
    pthread_mutex_unlock(&pool->_mutex_data);
}

int threadpool_add_task(struct threadpool *__restrict pool, 
                        struct task *task)
{
    int err;

    pthread_mutex_lock(&pool->_mutex_in);
    
    err = sem_post(&pool->_sem_in);
    if(err < 0) {
        err = -errno;
        goto out;
    }
    
    err = queue_enqueue(&pool->_queue_in, task, NULL);
    if(err < 0) {
        sem_wait(&pool->_sem_in);
        goto out;
    }
    
    pthread_mutex_lock(&pool->_mutex_data);
    
    pool->_tasks_inserted += 1;
    
    pthread_mutex_unlock(&pool->_mutex_data);
    
out:
    pthread_mutex_unlock(&pool->_mutex_in);
    
    return err;
}

struct task *threadpool_take_completed_task(struct threadpool *__restrict pool)
{
    struct task *task;
    int err;
    
again:
    err = sem_wait(&pool->_sem_out);
    if(err < 0) {
        if(errno == EINTR)
            goto again;
        else
            return NULL;
    }
    
    pthread_mutex_lock(&pool->_mutex_out);
    
    task = queue_dequeue(&pool->_queue_out);
    
    pool->_tasks_taken += 1;
    
    pthread_mutex_unlock(&pool->_mutex_out);
    
    return task;
}

bool threadpool_has_completed_task(struct threadpool *__restrict pool)
{
    bool ret;
    
    pthread_mutex_lock(&pool->_mutex_out);
    
    ret = queue_size(&pool->_queue_out) != 0;
    
    pthread_mutex_unlock(&pool->_mutex_out);
    
    return ret;
}

bool threadpool_done(struct threadpool *__restrict pool)
{
    bool ret;
    
    pthread_mutex_lock(&pool->_mutex_data);
    
    ret = pool->_tasks_taken == pool->_tasks_inserted;
    
    pthread_mutex_unlock(&pool->_mutex_data);
    
    return ret;
}

int threadpool_threads_available(struct threadpool *__restrict pool)
{
    int ret;
    
    pthread_mutex_lock(&pool->_mutex_data);
    
    ret = hash_size(&pool->_hash_threads);
    
    pthread_mutex_unlock(&pool->_mutex_data);
    
    return ret;
}