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

#ifndef _MAP_H_
#define _MAP_H_

#include <stdbool.h>

#define MAP_DEFAULT_LOWER_BOUND 10
#define MAP_DEFAULT_UPPER_BOUND 60
#define MAP_DEFAULT_SIZE 32

enum map_data_state {
    MAP_DATA_STATE_EMPTY        = 0x00,
    MAP_DATA_STATE_AVAILABLE    = 0x01,
    MAP_DATA_STATE_REMOVED      = 0x02
};

struct entry {
    const void *key;
    void *data;
    
    unsigned int hash;
    enum map_data_state state;
};

struct map_config {
    unsigned int size;
    
    unsigned int lower_bound;
    unsigned int upper_bound;
    
    bool static_size;
    
    int (*key_compare)(const void *, const void *);
    unsigned int (*key_hash)(const void *);
    void (*data_delete)(void *);
};

struct map {
    struct entry *table;
    unsigned int size;
    unsigned int capacity;
    
    unsigned int lower_bound;
    unsigned int upper_bound;
    
    bool static_size;

    int (*key_compare)(const void *, const void *);
    unsigned int (*key_hash)(const void *);
    void (*data_delete)(void *);
};

struct map *map_new(const struct map_config *__restrict conf);

void map_delete(struct map *__restrict map);

int map_init(struct map *__restrict map,
             const struct map_config *__restrict conf);

void map_destroy(struct map *__restrict map);

void map_clear(struct map *__restrict map);

int map_rehash(struct map *__restrict map, unsigned int size);

int map_insert(struct map *__restrict map, const void *key, void *data);

void *map_retrieve(struct map *__restrict map, const void *key);

void *map_take(struct map *__restrict map, const void *key);

bool map_contains(const struct map *__restrict map, const void *key);

inline unsigned int map_size(const struct map *__restrict map);

inline bool map_empty(const struct map *__restrict map);

void map_set_static_size(struct map *__restrict map, bool static_size);

bool map_static_size(const struct map *__restrict map);

inline const void *entry_key(struct entry *__restrict e);

inline void *entry_data(struct entry *__restrict e);


#define map_for_each(map, entry)                                               \
    for((entry) = (map)->table;                                                \
        (entry) < (map)->table + (map)->capacity;                              \
        (entry) += 1)                                                          \
        if((entry)->state != MAP_DATA_STATE_AVAILABLE)                         \
            continue;                                                          \
        else


#endif /* _MAP_H_ */