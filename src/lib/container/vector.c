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
#include <stdbool.h>
#include <errno.h>

#include "container_p.h"
#include "vector.h"

#define VECTOR_DEFAULT_CAPACITY 8

struct vector *vector_new(unsigned int capacity)
{
    struct vector *vec;
    int err;
    
    vec = malloc(sizeof(*vec));
    if(!vec)
        return NULL;
    
    err = vector_init(vec, capacity);
    if(err < 0) {
        free(vec);
        return NULL;
    }
    
    return vec;
}

void vector_delete(struct vector *__restrict vec)
{
    vector_destroy(vec);
    free(vec);
}

int vector_init(struct vector *__restrict vec, unsigned int capacity)
{
    capacity = adjust(capacity, VECTOR_DEFAULT_CAPACITY);
    
    vec->data = calloc(capacity, sizeof(*vec->data));
    if(!vec->data)
        return -errno;

    vec->size = 0;
    vec->capacity = capacity;
    vec->data_compare = NULL;
    vec->data_delete  = NULL;
    
    return 0;
}

void vector_destroy(struct vector *__restrict vec)
{
    vector_clear(vec);
    free(vec->data);
}

void vector_clear(struct vector *__restrict vec)
{
    if(!vec->data_delete) {
        vec->size = 0;
        return;
    }
    
    while(vec->size--)
        vec->data_delete(vec->data[vec->size]);
}

inline unsigned int vector_size(const struct vector *__restrict vec)
{
    return vec->size;
}

inline bool vector_empty(const struct vector *__restrict vec)
{
    return vec->size == 0;
}

int vector_set_capacity(struct vector *__restrict vec, unsigned int capacity)
{
    void **data;
    
    capacity = adjust(capacity, VECTOR_DEFAULT_CAPACITY);
    
    if(capacity == vec->capacity)
        return 0;
    
    data = realloc(vec->data, capacity * sizeof(*data));
    if(!data)
        return -errno;
    
    if(vec->size > capacity)
        vec->size = capacity;
    
    vec->data = data;
    vec->capacity = capacity;
    
    return 0;
}

inline unsigned int vector_capacity(const struct vector *__restrict vec)
{
    return vec->capacity;
}

int vector_squeeze(struct vector *__restrict vec)
{
    return vector_set_capacity(vec, vec->size);
}

int vector_insert_front(struct vector *__restrict vec, void *data)
{
    return vector_insert_at(vec, 0, data);
}

int vector_insert_at(struct vector *__restrict vec, unsigned int i, void *data)
{
    size_t move_size;
    int err;
    
    if(vec->size >= vec->capacity) {
        err = vector_set_capacity(vec, vec->capacity << 1);
        if(err < 0)
            return err;
    }
    
    move_size = (vec->size - i) * sizeof(*vec->data);
    
    memmove(vec->data + i + 1, vec->data + i, move_size);
    
    vec->data[i] = data;
    vec->size += 1;
    
    return 0;
}

int vector_insert_back(struct vector *__restrict vec, void *data)
{
    int err;
    
    if(vec->size >= vec->capacity) {
        err = vector_set_capacity(vec, vec->capacity << 1);
        if(err < 0)
            return err;
    }
    
    vec->data[vec->size] = data;
    vec->size += 1;
    
    return 0;
}

void *vector_take_front(struct vector *__restrict vec)
{
    return vector_take_at(vec, 0);
}

void *vector_take_at(struct vector *__restrict vec, unsigned int i)
{
    void *data;
    size_t move_size;
    
    data = vec->data[i];
    
    move_size = (vec->size - i) * sizeof(*vec->data);
    vec->size -= 1;
    
    memmove(vec->data + i, vec->data + i + 1, move_size);
    
    return data;
}

void *vector_take_back(struct vector *__restrict vec)
{
    vec->size -= 1;
    
    return vec->data[vec->size];
}

void *vector_take(struct vector *__restrict vec, void *data)
{
    unsigned int i;
    
    for(i = 0; i < vec->size; ++i) {
        if(vec->data[i] == data)
            return vector_take_at(vec, i);
    }
    
    return NULL;
}

void **vector_at(struct vector *__restrict vec, unsigned int i)
{
    return vec->data + i;
}

void **vector_front(struct vector *__restrict vec)
{
    return vec->data;
}

void **vector_back(struct vector *__restrict vec)
{
    return vec->data + vec->size - 1;
}

void vector_sort(struct vector *__restrict vec)
{
    qsort(vec->data, vec->size, sizeof(*vec->data), vec->data_compare);
}

int vector_insert_sorted(struct vector *__restrict vec, void *data)
{
    unsigned int i;
    int res;
    
    for(i = 0; i < vec->size; i++) {
        res = vec->data_compare(vec->data[i], data);
        
        if(res > 0)
            break;
    }
    
    return vector_insert_at(vec, i, data);
}

void *vector_take_sorted(struct vector *__restrict vec, void *data)
{
    int l, r, m;
    int res;
    
    l = 0;
    r = vec->size - 1;
    
    while(l <= r) {
        m = (l + r) >> 1;
        
        res = vec->data_compare(vec->data[m], data);
        if(res < 0)
            r = m - 1;
        else if(res > 0)
            l = m + 1;
        else
            return vector_take_at(vec, m);
    }
    
    return NULL;
}

void vector_set_data_compare(struct vector *__restrict vec, 
                             int (*data_compare)(const void *, const void *))
{
    vec->data_compare = data_compare;
}

int (*vector_data_compare(struct vector *__restrict vec))
                          (const void *, const void *)
{
    return vec->data_compare;
}

void vector_set_data_delete(struct vector *__restrict vec, 
                            void (*data_delete)(void *))
{
    vec->data_delete = data_delete;
}

void (*vector_data_delete(struct vector *__restrict vec))(void *)
{
    return vec->data_delete;
}