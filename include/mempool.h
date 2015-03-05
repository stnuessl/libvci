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

#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include <stdlib.h>
#include <stdbool.h>

struct mempool {
    void *mem;
    void *init;
    void *next;
    
    size_t size;
    size_t chunk_size;
    unsigned int chunks;
};

struct mempool *mempool_new(void *mem, size_t size, size_t chunk_size);

void mempool_delete(struct mempool *__restrict pool);

int mempool_init(struct mempool *__restrict pool, 
                 void *mem, 
                 size_t size, 
                 size_t chunk_size);

void mempool_destroy(struct mempool *__restrict pool);

void* mempool_alloc_chunk(struct mempool *__restrict pool);

void mempool_free_chunk(struct mempool *__restrict pool, void *chunk);

bool mempool_empty(const struct mempool *__restrict pool);

#endif /* _MEMPOOL_H_ */
