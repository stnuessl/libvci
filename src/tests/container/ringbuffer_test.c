#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include <ringbuffer.h>

struct ringbuffer *rb;

void *consumer(void *arg)
{
    int item, err;
    
    while(1) {
        err = ringbuffer_read(rb, &item, sizeof(item));
        assert(err == 0);
        
        fprintf(stdout, "Read: %d\n", item);
        
        pthread_testcancel();
    }
    
    return NULL;
}

void *producer(void *arg)
{
    int item, err;
    
    item = 1;
    
    while(1) {
        err = ringbuffer_write(rb, &item, sizeof(item));
        assert(err == 0);
        
        item += 1;
        
        pthread_testcancel();
    }
    
    return NULL;
}

#define N_THREADS 2
#define SLEEP_TIME 1

int main(int argc, char *argv[])
{
    pthread_t threads[N_THREADS];
    int i, err;
    
    rb = ringbuffer_new(1000);
    assert(rb);
    
    for(i = 0; i < N_THREADS; ++i) {
        if(i & 0x01)
            err = pthread_create(threads + i, NULL, &consumer, NULL);
        else
            err = pthread_create(threads + i, NULL, &producer, NULL);
        
        assert(err == 0);
    }
    
    sleep(SLEEP_TIME);
    
    for(i = 0; i < N_THREADS; ++i) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    
    ringbuffer_delete(rb);
    
    return EXIT_SUCCESS;
}