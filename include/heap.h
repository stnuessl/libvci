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

#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdbool.h>

struct heap {
    void **data;
    
    void (*data_delete)(void *);
    int (*data_compare)(const void *, const void *);
    
    unsigned int size;
    unsigned int capacity;
};


struct heap *heap_new(unsigned int capacity, 
                      int (*data_compare)(const void *, const void *));

void heap_delete(struct heap *__restrict heap);

int heap_init(struct heap *__restrict heap, 
              unsigned int capacity, 
              int (*data_compare)(const void *, const void *));

void heap_destroy(struct heap *__restrict heap);

void heap_clear(struct heap *__restrict heap);

int heap_insert(struct heap *__restrict heap, void *data);

void *heap_take(struct heap *__restrict heap);

void *heap_retrieve(struct heap *__restrict heap);

bool heap_empty(struct heap *__restrict heap);

unsigned int heap_size(struct heap *__restrict heap);

void heap_set_data_delete(struct heap *__restrict heap,
                          void (*data_delete)(void *));

void (*heap_data_delete(struct heap *__restrict heap))(void *);

void heap_set_data_compare(struct heap *__restrict heap,
                           int (*data_compare)(const void *, const void*));

int (*heap_data_compare(struct heap *__restrict heap))
                       (const void *, const void *);

#endif /* _HEAP_H_ */