
#ifndef _TASK_H_
#define _TASK_H_

#include <pthread.h>

struct task {
    void *_return_value;
    void *(*_func)(void *);
    void *_arg;
    
    void *_key;
    
    void (*_arg_delete)(void *);
};

struct task *task_new(void *(*func)(void *), void *arg);

void task_delete(struct task *__restrict task);

#define TASK_DEFINE_GET(name, type)                                            \
                                                                               \
inline type task_##name(struct task *__restrict task);

TASK_DEFINE_GET(return_value, void *)
TASK_DEFINE_GET(arg, void *)
TASK_DEFINE_GET(key, void *)

#undef TASK_DEFINE_GET

#define TASK_DEFINE_SET(name, type)                                            \
                                                                               \
inline void task_set_##name(struct task *__restrict task, type name);

TASK_DEFINE_SET(key, void *)

#undef TASK_DEFINE_SET

#define TASK_DEFINE_CALLBACK_GET(name, type, args)                             \
                                                                               \
inline type (*task_##name(struct task *__restrict task))args;

TASK_DEFINE_CALLBACK_GET(func, void *, (void *))

#undef TASK_DEFINE_CALLBACK_GET

#define TASK_DEFINE_CALLBACK_SET(name, type, args)                             \
                                                                               \
inline void task_set_##name(struct task *__restrict task, type (*name)args);

TASK_DEFINE_CALLBACK_SET(arg_delete, void, (void *))

#undef TASK_DEFINE_CALLBACK_SET

#endif /* _TASK_H_ */