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

#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdbool.h>

#include "link.h"

enum cache_data_state {
    CACHE_DATA_STATE_EMPTY      = 0x00,
    CACHE_DATA_STATE_AVAILABLE  = 0x01,
    CACHE_DATA_STATE_REMOVED    = 0x02
};

struct cache_entry {
    const void *key;
    void *data;
    
    struct link link;
    
    unsigned int hash;
    enum cache_data_state state;
};

struct cache {
    struct cache_entry *table;
    struct link list;
    
    unsigned int size;
    unsigned int max_size;
    unsigned int capacity;

    int (*key_compare)(const void *, const void *);
    unsigned int (*key_hash)(const void *);
    void (*data_delete)(void *);
};

struct cache *cache_new(unsigned int size,
                        int (*key_compare)(const void *, const void *),
                        unsigned int (*key_hash)(const void *));

void cache_delete(struct cache *__restrict cache);

int cache_init(struct cache *__restrict cache,
               unsigned int size,
               int (*key_compare)(const void *, const void *),
               unsigned int (*key_hash)(const void *));

void cache_destroy(struct cache *__restrict cache);

void cache_clear(struct cache *__restrict cache);

void cache_insert(struct cache *__restrict cache, const void *key, void *data);

void *cache_take(struct cache *__restrict cache, const void *key);

void *cache_retrieve(struct cache *__restrict cache, const void *key);

inline bool cache_empty(const struct cache *__restrict cache);

inline bool cache_full(const struct cache *__restrict cache);

inline unsigned int cache_size(const struct cache *__restrict cache);

inline unsigned int cache_capacity(const struct cache *__restrict cache);

inline void cache_set_data_delete(struct cache *__restrict cache,
                                  void (*data_delete)(void *));

#endif /* _CACHE_H_ */