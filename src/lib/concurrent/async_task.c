#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/eventfd.h>

#include "async_task.h"

static void *_async_task_run(void *arg)
{
    struct async_task *task;
    void *ret;
    
    task = arg;

    ret = task->func(task->arg);
    
    eventfd_write(task->event_fd, 1);
    
    pthread_exit(ret);
}

struct async_task *async_task_new(void *(*func)(void *))
{
    struct async_task *task;
    int err;
    
    task = malloc(sizeof(*task));
    if(!task)
        return NULL;
    
    err = async_task_init(task, func);
    if(err < 0) {
        free(task);
        return NULL;
    }
    
    return task;
}

void async_task_delete(struct async_task *__restrict task)
{
    async_task_destroy(task);
    free(task);
}

int async_task_init(struct async_task *__restrict task, 
                    void *(*func)(void *))
{
    task->event_fd = eventfd(0, 0);
    if(task->event_fd < 0)
        return -errno;
    
    task->func = func;
    
    return 0;
}

void async_task_destroy(struct async_task *__restrict task)
{
    close(task->event_fd);
}

int async_task_start(struct async_task *__restrict task, void *arg)
{
    pthread_attr_t attr;
    int err;

    err = pthread_attr_init(&attr);
    if(err)
        return -err;
    
    task->arg = arg;
    
    err = pthread_create(&task->id, &attr, &_async_task_run, task);
    
    pthread_attr_destroy(&attr);
    
    return (err) ? -err : 0;
}

void *async_task_finish(const struct async_task *__restrict task)
{
    void *ret;
    
    pthread_join(task->id, &ret);
    
    return ret;
}

int async_task_event_fd(const struct async_task *__restrict task)
{
    return task->event_fd;
}