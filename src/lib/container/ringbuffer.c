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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
    rb->data = malloc(capacity);
    if(!rb->data)
        return -errno;
    

    rb->size     = 0;
    rb->capacity = capacity;
    rb->i_read   = 0;
    rb->i_write  = 0;
    
    return 0;
}

void ringbuffer_destroy(struct ringbuffer *__restrict rb)
{
    free(rb->data);
}

void ringbuffer_write(struct ringbuffer *__restrict rb,
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


void ringbuffer_read(struct ringbuffer *__restrict rb, 
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

bool ringbuffer_empty(struct ringbuffer *__restrict rb)
{
    return rb->size == 0;
}

size_t ringbuffer_size(struct ringbuffer *__restrict rb)
{
    return rb->size;
}

size_t ringbuffer_capacity(struct ringbuffer *__restrict rb)
{
    return rb->capacity;
}
