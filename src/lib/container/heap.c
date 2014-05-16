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
#include <errno.h>
#include <stdbool.h>

#include "heap.h"
#include "container_p.h"
#include "macro.h"

#define HEAP_DEFAULT_CAPACITY 8

static void _heap_heapify_up(struct heap *__restrict heap)
{
    void *swap;
    unsigned int node, parent;
    int res;
    
    if(heap->size < 2)
        return;
        
    node = heap->size - 1;
    
    while(node > 0) {
        parent = (node - 1) >> 1;

        res = heap->data_compare(heap->data[parent], heap->data[node]);
        if(res > 0)
            break;
        
        swap               = heap->data[node];
        heap->data[node]   = heap->data[parent];
        heap->data[parent] = swap;

        node = parent;
    }
}

static void _heap_heapify_down(struct heap *__restrict heap)
{
    void *swap;
    unsigned int node, parent, left, right;
    int res;
    
    node = 0;
    
    while(1) {
        left   = (node << 1) + 1;
        right  = (node << 1) + 2;
        parent = node;
        
        if(left < heap->size) {
            res = heap->data_compare(heap->data[left], heap->data[parent]);
            if(res > 0)
                parent = left;
        }

        if(right < heap->size) {
            res = heap->data_compare(heap->data[right], heap->data[parent]);
            if(res > 0)
                parent = right;
        }
        
        if(node == parent)
            break;
        
        swap               = heap->data[parent];
        heap->data[parent] = heap->data[node];
        heap->data[node]   = swap;
        
        node = parent;
    }
}

static int _heap_resize(struct heap *__restrict heap, unsigned int capacity)
{
    void **data;
    
    capacity = max(HEAP_DEFAULT_CAPACITY, capacity);
    
    data = realloc(heap->data, capacity * sizeof(*data));
    if(!data)
        return -errno;
    
    heap->data = data;
    heap->capacity = capacity;
    
    return 0;
}

struct heap *heap_new(unsigned int capacity, 
                      int (*data_compare)(const void *, const void *))
{
    struct heap *heap;
    int err;
    
    heap = malloc(sizeof(*heap));
    if(!heap)
        return NULL;
    
    err = heap_init(heap, capacity, data_compare);
    if(err < 0) {
        free(heap);
        return NULL;
    }
    
    return heap;
}

void heap_delete(struct heap *__restrict heap)
{
    heap_destroy(heap);
    free(heap);
}

int heap_init(struct heap *__restrict heap, 
              unsigned int capacity, 
              int (*data_compare)(const void *, const void *))
{
    capacity = adjust(capacity, HEAP_DEFAULT_CAPACITY);
    
    heap->data = malloc(capacity * sizeof(*heap->data));
    if(!heap->data)
        return -errno;
    
    heap->size     = 0;
    heap->capacity = capacity;
    heap->data_compare = data_compare;
    heap->data_delete  = NULL;
    
    return 0;
}

void heap_destroy(struct heap *__restrict heap)
{
    heap_clear(heap);
    free(heap->data);
}

void heap_clear(struct heap *__restrict heap)
{
    if(!heap->data_delete) {
        heap->size = 0;
        return;
    }
    
    while(heap->size--)
        heap->data_delete(heap->data[heap->size]);
}

int heap_insert(struct heap *__restrict heap, void *data)
{
    int err;
    
    if(heap->size >= heap->capacity) {
        err = _heap_resize(heap, heap->capacity << 1);
        if(err < 0)
            return err;
    }
    
    heap->data[heap->size] = data;
    heap->size += 1;
    
    _heap_heapify_up(heap);
    
    return 0;
}

void *heap_take(struct heap *__restrict heap)
{
    void *ret;
    
    ret = heap->data[0];
    
    heap->size -= 1;
    
    heap->data[0] = heap->data[heap->size];
    
    _heap_heapify_down(heap);
    
    if(heap->size < (heap->capacity >> 1))
        _heap_resize(heap, heap->capacity >> 1);
    
    return ret;
}

void *heap_retrieve(struct heap *__restrict heap)
{
    return heap->data[0];
}

bool heap_empty(struct heap *__restrict heap)
{
    return heap->size == 0;
}

unsigned int heap_size(struct heap *__restrict heap)
{
    return heap->size;
}

void heap_set_data_delete(struct heap *__restrict heap,
                          void (*data_delete)(void *))
{
    heap->data_delete = data_delete;
}

void (*heap_data_delete(struct heap *__restrict heap))(void *)
{
    return heap->data_delete;
}

void heap_set_data_compare(struct heap *__restrict heap,
                           int (*data_compare)(const void *, const void*))
{
    heap->data_compare = data_compare;
}

int (*heap_data_compare(struct heap *__restrict heap))
                       (const void *, const void *)
{
    return heap->data_compare;
}