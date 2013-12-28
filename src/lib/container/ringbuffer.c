#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <stdbool.h>

#include "ringbuffer.h"

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
    
    err = sem_init(&rb->sem_write, 0, 0);
    if(err < 0) {
        err = -errno;
        goto cleanup2;
    }
    
    err = sem_init(&rb->sem_read, 0, 0);
    if(err < 0) {
        err = -errno;
        goto cleanup3;
    }
    
    rb->read_blocked  = false;
    rb->write_blocked = false;
    
    rb->size     = 0;
    rb->capacity = capacity;
    rb->i_read   = 0;
    rb->i_write  = 0;
    
    return 0;

cleanup3:
    sem_destroy(&rb->sem_write);
cleanup2:
    pthread_mutex_destroy(&rb->mutex);
cleanup1:
    free(rb->data);
out:
    return err;
}

void ringbuffer_destroy(struct ringbuffer *__restrict rb)
{
    sem_destroy(&rb->sem_read);
    sem_destroy(&rb->sem_write);
    
    pthread_mutex_destroy(&rb->mutex);
    free(rb->data);
}

int ringbuffer_write(struct ringbuffer *__restrict rb,
                     const void *__restrict data,
                     size_t size)
{
    size_t new_i_write, first_copy_size, second_copy_size;
    int err;
    
    /* 
     * Why isn't there a ready to use implementation of counting semaphores?
     */
    pthread_mutex_lock(&rb->mutex);
    
    while(rb->size + size >= rb->capacity) {
        rb->write_blocked = true;
        pthread_mutex_unlock(&rb->mutex);

again:
        err = sem_wait(&rb->sem_write);
        if(err < 0) {
            if(errno == EINTR)
                goto again;
            else
                return -errno;
        }
        
        pthread_mutex_lock(&rb->mutex);
    }

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
    
    if(rb->read_blocked) {
        err = sem_post(&rb->sem_read);
        if(err < 0) {
            pthread_mutex_unlock(&rb->mutex);
            return -errno;
        }
        
        rb->read_blocked = false;
    }
    
    pthread_mutex_unlock(&rb->mutex);
    return 0;
}

int ringbuffer_read(struct ringbuffer *__restrict rb, void *data, size_t size)
{
    size_t new_i_read, first_copy_size, second_copy_size;
    int err;
    
    pthread_mutex_lock(&rb->mutex);
    
    while(size > rb->size) {
        rb->read_blocked = true;
        pthread_mutex_unlock(&rb->mutex);
        
again:
        err = sem_wait(&rb->sem_read);
        if(err < 0) {
            if(errno == EINTR)
                goto again;
            else
                return -errno;
        }
        
        pthread_mutex_lock(&rb->mutex);
    }
    
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
    rb->size -= size;
    
    if(rb->write_blocked) {
        err = sem_post(&rb->sem_write);
        if(err < 0) {
            pthread_mutex_unlock(&rb->mutex);
            return -errno;
        }
        
        rb->write_blocked = false;
    }

    pthread_mutex_unlock(&rb->mutex);
    return 0;
}

int ringbuffer_try_read(struct ringbuffer *__restrict rb, 
                        void *data, 
                        size_t size)
{
    bool ret;
    
    pthread_mutex_lock(&rb->mutex);
    
    ret = rb->size > 0;
    
    pthread_mutex_unlock(&rb->mutex);
    
    return ret;
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