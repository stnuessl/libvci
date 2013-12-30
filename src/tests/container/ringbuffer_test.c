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

void *consumer(void *arg)
{
    while(1) {
        if(ringbuffer_size(arg) >= sizeof(last_item))
            ringbuffer_read(arg, &last_item, sizeof(last_item));
        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

void *producer(void *arg)
{
    int item;
    
    item = 1;
    
    while(1) {
        ringbuffer_write(arg, &item, sizeof(item));
        
        item += 1;
        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

void test_blocking(void)
{
    struct ringbuffer *rb;
    pthread_t threads[N_THREADS];
    int i, err;
    
    rb = ringbuffer_new(RINGBUFFER_SIZE);
    assert(rb);
    
    for(i = 0; i < N_THREADS; ++i) {
        if(i & 0x01)
            err = pthread_create(threads + i, NULL, &consumer, rb);
        else
            err = pthread_create(threads + i, NULL, &producer, rb);
        
        assert(err == 0);
    }
    
    sleep(SLEEP_TIME);
    
    for(i = 0; i < N_THREADS; ++i) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    
    fprintf(stdout, "Last item (blocking): %d\n", last_item);
    
    ringbuffer_delete(rb);
}

int main(int argc, char *argv[])
{
    test_blocking();
    
    return EXIT_SUCCESS;
}