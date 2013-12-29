#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

struct ringbuffer {
    char *data;
    
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_write_cond;
    pthread_mutex_t mutex_read_cond;
    
    pthread_cond_t cond_write;
    pthread_cond_t cond_read;
    
    bool write_blocked;
    bool read_blocked;
    
    size_t size;
    size_t capacity;
    
    size_t i_read;
    size_t i_write;
};

struct ringbuffer *ringbuffer_new(size_t capacity);

void ringbuffer_delete(struct ringbuffer *__restrict rb);

int ringbuffer_init(struct ringbuffer *__restrict rb, size_t capacity);

void ringbuffer_destroy(struct ringbuffer *__restrict rb);

int ringbuffer_write(struct ringbuffer *__restrict rb, 
                     const void *__restrict data, 
                     size_t size);

int ringbuffer_try_write(struct ringbuffer *__restrict rb,
                         const void *__restrict data,
                         size_t size);

int ringbuffer_read(struct ringbuffer *__restrict rb,
                    void *__restrict data, 
                    size_t size);

int ringbuffer_try_read(struct ringbuffer *__restrict rb, 
                        void *__restrict data, 
                        size_t size);

bool ringbuffer_empty(struct ringbuffer *__restrict rb);

size_t ringbuffer_size(struct ringbuffer *__restrict rb);

size_t ringbuffer_capacity(struct ringbuffer *__restrict rb);

#endif /* _RINGBUFFER_H_ */