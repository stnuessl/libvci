#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "threadpool_task.h"
#include "macro.h"


void threadpool_task_set_function(struct threadpool_task *__restrict task, 
                                  void (*func)(struct threadpool_task *))
{
    task->func = func;
}

void (*threadpool_task_function(struct threadpool_task *__restrict task))
                               (struct threadpool_task *)
{
    return task->func;
}