#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "macros.h"

struct task *task_new(void *(*func)(void *), void *arg)
{
    struct task *task;
    
    task = malloc(sizeof(*task));
    if(!task)
        return NULL;
    
    memset(task, 0, sizeof(*task));
    
    task->func = func;
    task->arg  = arg;
    
    return task;
}

void task_delete(struct task *__restrict task)
{
    free(task);
}

void task_delete_by_link(struct link *link)
{
    task_delete(container_of(link, struct task, link));
}

inline void task_set_key(struct task *__restrict task, void *key)
{
    task->key = key;
}

inline void *task_key(struct task *__restrict task)
{
    return task->key;
}

inline void *task_return_value(struct task *__restrict task)
{
    return task->ret_val;
}