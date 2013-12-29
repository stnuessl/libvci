#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include <ringbuffer.h>


#define N_THREADS 2
#define SLEEP_TIME 1
#define RINGBUFFER_SIZE 1000

int last_item;

void *consumer(void *arg)
{
    int err;
    
    while(1) {
        err = ringbuffer_read(arg, &last_item, sizeof(last_item));
        assert(err == 0);
        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

void *producer(void *arg)
{
    int item, err;
    
    item = 1;
    
    while(1) {
        err = ringbuffer_write(arg, &item, sizeof(item));
        assert(err == 0);
        
        item += 1;
        
        pthread_testcancel();
        pthread_yield();
    }
    
    return NULL;
}

void *trying_consumer(void *arg)
{
    int err;
    
    while(1) {
        pthread_testcancel();
        err = ringbuffer_try_read(arg, &last_item, sizeof(last_item));
        if(err == EBUSY)
            continue;
        
        assert(err == 0);

        pthread_yield();
    }
    
    return NULL;
}

void *trying_producer(void *arg)
{
    int item, err;
    
    item = 1;
    
    while(1) {
        pthread_testcancel();
        err = ringbuffer_try_write(arg, &item, sizeof(item));
        if(err == EBUSY)
            continue;
        
        assert(err == 0);
        
        item += 1;
        
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

void test_nonblocking(void)
{
    struct ringbuffer *rb_try;
    pthread_t threads[N_THREADS];
    int i, err;
    
    rb_try = ringbuffer_new(RINGBUFFER_SIZE);
    assert(rb_try);
    
    for(i = 0; i < N_THREADS; ++i) {
        if(i & 0x01)
            err = pthread_create(threads + i, NULL, &trying_consumer, rb_try);
        else
            err = pthread_create(threads + i, NULL, &trying_producer, rb_try);
        
        assert(err == 0);
    }
    
    sleep(SLEEP_TIME);
    
    for(i = 0; i < N_THREADS; ++i) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    
    fprintf(stdout, "Last item (nonblocking): %d\n", last_item);
    
    ringbuffer_delete(rb_try);
}

int main(int argc, char *argv[])
{
    test_blocking();
    test_nonblocking();

    return EXIT_SUCCESS;
}