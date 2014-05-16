/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <libvci/ringbuffer.h>

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