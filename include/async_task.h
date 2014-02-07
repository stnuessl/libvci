#ifndef _ASYNC_TASK_H_
#define _ASYNC_TASK_H_

struct async_task {
    pthread_t id;
    void *(*func)(void *);

    void *arg;
    int event_fd;
};

struct async_task *async_task_new(void *(*func)(void *));

void async_task_delete(struct async_task *__restrict task);

int async_task_init(struct async_task *__restrict task, 
                    void *(*func)(void *));

void async_task_destroy(struct async_task *__restrict task);

int async_task_start(struct async_task *__restrict task, void *arg);

void *async_task_finish(const struct async_task *__restrict task);

int async_task_event_fd(const struct async_task *__restrict task);

#endif /* _ASYNC_TASK_H_ */