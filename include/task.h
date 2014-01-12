#ifndef _TASK_H_
#define _TASK_H_

#include <pthread.h>
#include <stdbool.h>

#include "link.h"

struct task {
    struct link link;
    
    void (*func)(struct task *);
    void *val;
    void *ret;
};

void task_set_function(struct task *__restrict task, 
                       void (*func)(struct task *));

void (*task_function(struct task *__restrict task))(struct task *);

void task_set_value(struct task *__restrict task, void *val);

void *task_value(struct task *__restrict task);

void task_set_return_value(struct task *__restrict task, void *ret);

void *task_return_value(struct task *__restrict task);

#endif /* _TASK_H_ */