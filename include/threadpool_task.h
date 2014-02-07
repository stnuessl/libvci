#ifndef _THREADPOOL_TASK_H_
#define _THREADPOOL_TASK_H_

#include <pthread.h>
#include <stdbool.h>

#include "link.h"

struct threadpool_task {
    struct link link;
    void (*func)(struct threadpool_task *);
};

void threadpool_task_set_function(struct threadpool_task *__restrict task, 
                                  void (*func)(struct threadpool_task *));

void (*threadpool_task_function(struct threadpool_task *__restrict task))
                               (struct threadpool_task *);

#endif /* _THREADPOOL_TASK_H_ */