#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"


struct task *task_new(void *(*func)(void *), void *arg)
{
    struct task *task;
    
    task = malloc(sizeof(*task));
    if(!task)
        return NULL;
    
    memset(task, 0, sizeof(*task));
    
    task->_func = func;
    task->_arg  = arg;
    
    return task;
}

void task_delete(struct task *__restrict task)
{   
    if(task->_arg_delete)
        task->_arg_delete(task->_arg);
    
    free(task);
}

#define TASK_DEFINE_GET(name, type)                                            \
                                                                               \
inline type task_##name(struct task *__restrict task)                          \
{                                                                              \
    return task->_##name;                                                      \
}

TASK_DEFINE_GET(return_value, void *)
TASK_DEFINE_GET(arg, void *)
TASK_DEFINE_GET(key, void *)

#undef TASK_DEFINE_GET

#define TASK_DEFINE_SET(name, type)                                            \
                                                                               \
inline void task_set_##name(struct task *__restrict task, type name)           \
{                                                                              \
    task->_##name = name;                                                      \
}

TASK_DEFINE_SET(key, void *)

#undef TASK_DEFINE_SET

#define TASK_DEFINE_CALLBACK_GET(name, type, args)                             \
                                                                               \
inline type (*task_##name(struct task *__restrict task))args                   \
{                                                                              \
    return task->_##name;                                                      \
}

TASK_DEFINE_CALLBACK_GET(func, void *, (void *))

#undef TASK_DEFINE_CALLBACK_GET

#define TASK_DEFINE_CALLBACK_SET(name, type, args)                             \
                                                                               \
inline void task_set_##name(struct task *__restrict task, type (*name)args)    \
{                                                                              \
    task->_##name = name;                                                      \
}

TASK_DEFINE_CALLBACK_SET(arg_delete, void, (void *))

#undef TASK_DEFINE_CALLBACK_SET