#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <ringbuffer.h>

#define N_THREADS 2
#define SLEEP_TIME 1
#define RINGBUFFER_SIZE 100000

int last_item;

struct thread_arg {
    struct ringbuffer rb;
    pthread_mutex_t mutex;
};

void *consumer(void *arg)
{
    struct thread_arg *tmp;

    tmp= arg;
    
    while(1) {
        pthread_mutex_lock(&tmp->mutex);
        if(ringbuffer_size(arg) >= sizeof(last_item))
            ringbuffer_read(arg, &last_item, sizeof(last_item));
        pthread_mutex_unlock(&tmp->mutex);

        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

void *producer(void *arg)
{
    struct thread_arg *tmp; 
    int item;
    
    tmp = arg;
    item = 1;
    
    while(1) {
        pthread_mutex_lock(&tmp->mutex);
        ringbuffer_write(&tmp->rb, &item, sizeof(item));
        pthread_mutex_unlock(&tmp->mutex);
        
        item += 1;
        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    struct thread_arg ta;
    pthread_t threads[N_THREADS];
    int i, err;
    
    err = ringbuffer_init(&ta.rb, RINGBUFFER_SIZE);
    assert(err == 0);
    err = pthread_mutex_init(&ta.mutex, NULL);
    assert(err == 0);
    
    for(i = 0; i < N_THREADS; ++i) {
        if(i & 0x01)
            err = pthread_create(threads + i, NULL, &consumer, &ta);
        else
            err = pthread_create(threads + i, NULL, &producer, &ta);
        
        assert(err == 0);
    }
    
    sleep(SLEEP_TIME);
    
    for(i = 0; i < N_THREADS; ++i) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    
    fprintf(stdout, "Last item: %d\n", last_item);
    
    ringbuffer_destroy(&ta.rb);
    pthread_mutex_destroy(&ta.mutex);
    
    return EXIT_SUCCESS;
}