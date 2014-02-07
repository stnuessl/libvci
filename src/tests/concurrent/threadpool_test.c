#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <libvci/threadpool_task.h>
#include <libvci/threadpool.h>
#include <libvci/macro.h>

#define MAX_EPOLL_EVENTS 10

struct my_task {
    struct threadpool_task task;
    unsigned int id;
};

void my_task_run(struct threadpool_task *task)
{
    struct my_task *my_task;
    
    my_task = container_of(task, struct my_task, task);

    my_task->id = 0xffff;
}

static const char _usage[] = {
    "Usage  : %s <number of threads> <number of tasks>\n"
    "Example: %s 10 100\n"
};

struct threadpool *pool;
struct epoll_event epoll_events[MAX_EPOLL_EVENTS];
int epoll_fd;

void epoll_init(void)
{
    struct epoll_event ev;
    int err;
    
    epoll_fd = epoll_create(1);
    assert(epoll_fd >= 0);
    
    memset(&ev.data, 0, sizeof(ev.data));
    
    ev.data.fd  = threadpool_event_fd(pool);
    ev.events   = EPOLLIN;
    
    err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    assert(err == 0);
}

void insert_tasks(int num_tasks)
{
    struct my_task *my_task;
    int i, err;
    
    for(i = 0; i < num_tasks; ++i) {
        my_task = malloc(sizeof(*my_task));
        assert(my_task);

        threadpool_task_set_function(&my_task->task, &my_task_run);
        
        err = threadpool_add_task(pool, &my_task->task);
        assert(err == 0);
    }
}

void test_adding_removing_threads(void)
{
    struct threadpool *pool;
    int threads, i, err;
    
    threads = 4;
    
    pool = threadpool_new(threads);
    assert(pool);
    
    for(i = 0; i< threads; ++i) {
        err = threadpool_add_thread(pool);
        assert(err == 0);
    }
    
    for(i = 0; i < threads; ++i) {
        err = threadpool_remove_thread(pool);
        assert(err == 0);
    }
    
    for(i = 0; i < threads; ++i) {
        err = threadpool_add_thread(pool);
        assert(err == 0);
        err = threadpool_remove_thread(pool);
        assert(err == 0);
    }

    threadpool_delete(pool);
}

void test_usage(int argc, char *argv[])
{
    struct my_task *my_task;
    struct threadpool_task *task;
    eventfd_t tasks_done;
    int i, err, num_threads, num_tasks, fds;
    
    if(argc != 3) {
        fprintf(stdout, _usage, argv[0], argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    num_threads = atoi(argv[1]);
    num_tasks   = atoi(argv[2]);
    
    pool = threadpool_new(num_threads);
    assert(pool);
    
    epoll_init();
    insert_tasks(num_tasks);
    
    for(i = 0; i < num_tasks;) {
        fds = epoll_wait(epoll_fd, epoll_events, MAX_EPOLL_EVENTS, -1);
        
        while(fds--) {
            assert(epoll_events[fds].data.fd == threadpool_event_fd(pool));
            
            err = eventfd_read(epoll_events[fds].data.fd, &tasks_done);
            assert(err == 0);

            i += tasks_done;
            
            while(tasks_done--) {
                task = threadpool_take_completed_task(pool);
                
                my_task = container_of(task, struct my_task, task);
                
                assert(my_task->id == 0xffff);
                free(my_task);
            }
        }
    }
    
    close(epoll_fd);
    threadpool_delete(pool);
}

int main(int argc, char *argv[])
{
    test_adding_removing_threads();
    test_usage(argc, argv);
    
    fprintf(stdout, 
            "Tests finished. %s threads executed %s tasks\n", 
            argv[1], argv[2]);
    
    return EXIT_SUCCESS;
}

