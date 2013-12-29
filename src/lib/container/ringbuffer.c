#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <stdbool.h>

#include "ringbuffer.h"

static void _ringbuffer_read(struct ringbuffer *__restrict rb,
                             void *__restrict data, 
                             size_t size)
{
    size_t new_i_read, first_copy_size, second_copy_size;
    
    new_i_read = rb->i_read + size;
    
    if(new_i_read >= rb->capacity) {
        new_i_read %= rb->capacity;
        
        first_copy_size = rb->capacity - rb->i_read;
        second_copy_size = size - first_copy_size;
        
        memcpy(data, rb->data + rb->i_read, first_copy_size);
        memcpy(data + first_copy_size, rb->data, second_copy_size);
    } else {
        memcpy(data, rb->data + rb->i_read, size);
    }
    
    rb->i_read = new_i_read;
    rb->size  -= size;
}

static void _ringbuffer_write(struct ringbuffer *__restrict rb,
                              const void *__restrict data,
                              size_t size)
{
    size_t new_i_write, first_copy_size, second_copy_size;
    
    new_i_write = rb->i_write + size;
    
    if(new_i_write >= rb->capacity) {
        new_i_write %= rb->capacity;
        
        first_copy_size  = rb->capacity - rb->i_write;
        second_copy_size = size - first_copy_size;
        
        memcpy(rb->data + rb->i_write, data, first_copy_size);
        memcpy(rb->data, data + first_copy_size, second_copy_size);
    } else {
        memcpy(rb->data + rb->i_write, data, size);
    }
    
    rb->i_write = new_i_write;
    rb->size += size;
}

struct ringbuffer *ringbuffer_new(size_t capacity)
{
    struct ringbuffer *rb;
    int err;
    
    rb = malloc(sizeof(*rb));
    if(!rb)
        return NULL;
    
    err = ringbuffer_init(rb, capacity);
    if(err < 0) {
        free(rb);
        return NULL;
    }
    
    return rb;
}

void ringbuffer_delete(struct ringbuffer *__restrict rb)
{
    ringbuffer_destroy(rb);
    free(rb);
}

int ringbuffer_init(struct ringbuffer *__restrict rb, size_t capacity)
{
    int err;
    
    rb->data = malloc(capacity);
    if(!rb->data) {
        err = -errno;
        goto out;
    }
    
    err = pthread_mutex_init(&rb->mutex, NULL);
    if(err) {
        errno = err;
        err = -errno;
        goto cleanup1;
    }
    
    err = pthread_mutex_init(&rb->mutex_write_cond, NULL);
    if(err) {
        errno = err;
        err = -errno;
        goto cleanup2;
    }
    
    err = pthread_mutex_init(&rb->mutex_read_cond, NULL);
    if(err) {
        errno = err;
        err = -errno;
        goto cleanup3;
    }
    
    err = pthread_cond_init(&rb->cond_write, NULL);
    if(err) {
        errno = err;
        err = -errno;
        goto cleanup4;
    }
    
    err = pthread_cond_init(&rb->cond_read, NULL);
    if(err) {
        errno = err;
        err = -errno;
        goto cleanup5;
    }
    
    rb->size     = 0;
    rb->capacity = capacity;
    rb->i_read   = 0;
    rb->i_write  = 0;
    
    return 0;

cleanup5:
    pthread_cond_destroy(&rb->cond_write);
cleanup4:
    pthread_mutex_destroy(&rb->mutex_read_cond);
cleanup3:
    pthread_mutex_destroy(&rb->mutex_write_cond);
cleanup2:
    pthread_mutex_destroy(&rb->mutex);
cleanup1:
    free(rb->data);
out:
    return err;
}

void ringbuffer_destroy(struct ringbuffer *__restrict rb)
{
    pthread_cond_destroy(&rb->cond_read);
    pthread_cond_destroy(&rb->cond_write);
    
    pthread_mutex_destroy(&rb->mutex_read_cond);
    pthread_mutex_destroy(&rb->mutex_write_cond);    
    pthread_mutex_destroy(&rb->mutex);
    
    free(rb->data);
}

int ringbuffer_write(struct ringbuffer *__restrict rb,
                     const void *__restrict data,
                     size_t size)
{
    int err;
    
    pthread_mutex_lock(&rb->mutex);
    
    while(rb->size + size >= rb->capacity) {
        rb->write_blocked = true;
        pthread_mutex_unlock(&rb->mutex);

        pthread_mutex_lock(&rb->mutex_write_cond);
        err = pthread_cond_wait(&rb->cond_write, &rb->mutex_write_cond);
        pthread_mutex_unlock(&rb->mutex_write_cond);
        
        if(err)
            return -err;
        
        pthread_mutex_lock(&rb->mutex);
    }
    _ringbuffer_write(rb, data, size);
    
    pthread_mutex_unlock(&rb->mutex);
    
    pthread_cond_broadcast(&rb->cond_read);
    return 0;
}

int ringbuffer_try_write(struct ringbuffer *__restrict rb,
                         const void *__restrict data,
                         size_t size)
{
    pthread_mutex_lock(&rb->mutex);
    
    if(rb->size + size >= rb->capacity) {
        pthread_mutex_unlock(&rb->mutex);
        return EBUSY;
    }
    
    _ringbuffer_write(rb, data, size);
    
    pthread_mutex_unlock(&rb->mutex);
    
    pthread_cond_broadcast(&rb->cond_read);
    return 0;
}

int ringbuffer_read(struct ringbuffer *__restrict rb, 
                    void *__restrict data, 
                    size_t size)
{
    int err;
    
    pthread_mutex_lock(&rb->mutex);
    
    while(size > rb->size) {
        pthread_mutex_unlock(&rb->mutex);
        
        pthread_mutex_lock(&rb->mutex_read_cond);
        err = pthread_cond_wait(&rb->cond_read, &rb->mutex_read_cond);
        pthread_mutex_unlock(&rb->mutex_read_cond);
        
        if(err)
            return -err;
        
        pthread_mutex_lock(&rb->mutex);
    }
    
    _ringbuffer_read(rb, data, size);
    
    pthread_mutex_unlock(&rb->mutex);

    pthread_cond_broadcast(&rb->cond_write);
    return 0;
}

int ringbuffer_try_read(struct ringbuffer *__restrict rb, 
                        void *data, 
                        size_t size)
{
    pthread_mutex_lock(&rb->mutex);
    
    if(rb->size < size) {
        pthread_mutex_unlock(&rb->mutex);
        return EBUSY;
    }
    
    _ringbuffer_read(rb, data, size);
    
    pthread_mutex_unlock(&rb->mutex);
    
    pthread_cond_broadcast(&rb->cond_write);
    return 0;
}

bool ringbuffer_empty(struct ringbuffer *__restrict rb)
{
    bool ret;
    
    pthread_mutex_lock(&rb->mutex);
    ret = rb->size == 0;
    pthread_mutex_unlock(&rb->mutex);
    
    return ret;
}

size_t ringbuffer_size(struct ringbuffer *__restrict rb)
{
    size_t ret;
    
    pthread_mutex_lock(&rb->mutex);
    ret = rb->size;
    pthread_mutex_unlock(&rb->mutex);
    
    return ret;
}

size_t ringbuffer_capacity(struct ringbuffer *__restrict rb)
{
    size_t ret;
    
    pthread_mutex_lock(&rb->mutex);
    ret = rb->capacity;
    pthread_mutex_unlock(&rb->mutex);
    
    return ret;
}