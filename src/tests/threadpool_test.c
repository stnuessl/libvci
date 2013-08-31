#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <task.h>
#include <threadpool.h>


void *my_task(void *arg)
{
    fprintf(stdout, "Task%li doing some work...\n", (long) arg);
    
    return arg;
}

static const char _usage[] = {
    "Usage  : %s <number of threads> <number of tasks>\n"
    "Example: %s 10 100\n"
};

int main(int argc, char *argv[])
{
    struct threadpool pool;
    struct task *task;
    long i;
    int err, num_threads, num_tasks;
    
    if(argc != 3) {
        fprintf(stdout, _usage, argv[0], argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    num_threads = atoi(argv[1]);
    num_tasks   = atoi(argv[2]);
    
    err = threadpool_init(&pool, num_threads);
    if(err < 0) {
        fprintf(stderr, "%d: %s\n", -err, strerror(-err));
        abort();
    }
    
    for(i = 0; i < num_tasks; ++i) {
        task = task_new(&my_task, (void *) i);
        assert(task);
        
        task_set_key(task, (void *)i);
        
        err = threadpool_add_task(&pool, task);
        if(err < 0)
            abort();
    }
    
    err = threadpool_add_thread(&pool);
    if(err < 0)
        abort();
    
    threadpool_remove_thread(&pool);
    
    while(!threadpool_done(&pool)) {
        task = threadpool_take_completed_task(&pool);
        
        fprintf(stdout, "Task%li finished with %li.\n",
                (long) task_key(task), 
                (long) task_return_value(task));
        
        task_delete(task);
    }

    threadpool_destroy(&pool);
    
    /*
     * it seems that the threads need actually some time to clean themselves up
     * a small sleep makes valgrind (and me) happy ;-)
     */
    sleep(1);
    
    return EXIT_SUCCESS;
}

