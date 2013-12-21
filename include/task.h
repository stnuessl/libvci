#ifndef _TASK_H_
#define _TASK_H_

#include <pthread.h>
#include <stdbool.h>

#include "link.h"

struct task {
    void *(*func)(void *);
    void *arg;
    
    void *ret_val;
    void *key;
    
    struct link link;
};

struct task *task_new(void *(*func)(void *), void *arg);

void task_delete(struct task *__restrict task);

void task_delete_by_link(struct link *link);

inline void task_set_key(struct task *__restrict task, void *key);

inline void *task_key(struct task *__restrict task);

inline void *task_return_value(struct task *__restrict task);

#endif /* _TASK_H_ */