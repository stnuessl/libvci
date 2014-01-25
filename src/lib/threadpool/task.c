#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "macro.h"


void task_set_function(struct task *__restrict task, 
                       void (*func)(struct task *))
{
    task->func = func;
}

void (*task_function(struct task *__restrict task))(struct task *)
{
    return task->func;
}

void task_set_value(struct task *__restrict task, void *val)
{
    task->val = val;
}

void *task_value(struct task *__restrict task)
{
    return task->val;
}

void task_set_return_value(struct task *__restrict task, void *ret)
{
    task->ret = ret;
}

void *task_return_value(struct task *__restrict task)
{
    return task->ret;
}