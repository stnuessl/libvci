#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <task.h>
#include <threadpool.h>

#define MAX_EPOLL_EVENTS 10

void *my_task(void *arg)
{
    fprintf(stdout, "Task%li running...\n", (long) arg);

    return arg;
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
    
    ev.data.ptr = pool;
    ev.data.fd  = threadpool_event_fd(pool);
    ev.events   = EPOLLIN;
    
    err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    assert(err == 0);
}

void insert_tasks(int num_tasks)
{
    struct task *task;
    int i, err;
    
    for(i = 0; i < num_tasks; ++i) {
        task = task_new(&my_task, (void *)(long) i, true);
        assert(task);
        
        task_set_key(task, (void *)(long)i);
        
        err = threadpool_add_task(pool, task);
        assert(err == 0);
    }
}

int main(int argc, char *argv[])
{
    struct task *task;
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
            
            fprintf(stdout, "INFO: %lu tasks were completed.\n", tasks_done);
            
            i += tasks_done;
            
            while(tasks_done--) {
                task = threadpool_take_completed_task(pool);
                
                fprintf(stdout, "Task%d finished with %d.\n",
                        (int)(long) task_key(task), 
                        (int)(long) task_return_value(task));
                
                task_delete(task);
            }
        }
    }

    close(epoll_fd);
    threadpool_delete(pool);
    
    /*
     * it seems that the threads need actually some time to clean themselves up
     * a small sleep makes valgrind (and me) happy ;-)
     * That's why I don't use  __on_return()-macro.
     */
    sleep(1);
    
    return EXIT_SUCCESS;
}

