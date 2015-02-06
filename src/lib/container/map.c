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
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "map.h"
#include "container_p.h"
#include "macro.h"

static inline bool map_should_grow(const struct map *__restrict map)
{
    return 100 * map->size / map->capacity >= map->upper_bound;
}

static inline bool map_should_shrink(const struct map *__restrict map)
{
    return 100 * map->size / map->capacity < map->lower_bound;
}

static int map_resize(struct map *__restrict map, unsigned int capacity)
{
    struct entry *old_table;
    unsigned int i, old_capacity, old_size;
    int err;
    
    old_size     = map->size;
    old_capacity = map->capacity;
    old_table    = map->table;
    
    map->size     = 0;
    map->capacity = max(capacity, MAP_DEFAULT_SIZE);
    map->table    = calloc(map->capacity, sizeof(*map->table));

    if (!map->table) {
        err = -errno;
        goto out;
    }
    
    for (i = 0; i < old_capacity; ++i) {
        if (old_table[i].state == MAP_DATA_STATE_AVAILABLE) {
            err = map_insert(map, old_table[i].key, old_table[i].data);
            if(err < 0)
                goto cleanup1;
        }
    }
    
    free(old_table);
    
    return 0;

cleanup1:
    free(map->table);
out:
    map->size     = old_size;
    map->capacity = old_capacity;
    map->table    = old_table;

    return err;
}

/*
 * Search for an map_entry with the given 'key'.
 * If the state field of the map_entry states
 * 'DATA_EMPTY' we couldn't find an entry with 'key'.
 * If the state is 'DATA_AVAILABLE' we have to check if this is
 * the data we are searching for.
 * If it isn't we keep searching for it on the next possible position.
 * If state is 'DATA_REMOVED' it also means we have to search on the 
 * next possible position.
 * 
 * Note:
 * If there is a bug in this function it is very likely that
 * map_insert() suffers from the same bug.
 */
static struct entry *map_lookup(const struct map *__restrict map,
                                const void *__restrict key)
{
    unsigned int hash, index, offset;
    
    hash = map->key_hash(key);

    index = hash & (map->capacity - 1);
    offset = 1;
    
    while(offset < map->capacity) {

        switch(map->table[index].state) {
        case MAP_DATA_STATE_AVAILABLE:
            if(map->table[index].hash != hash)
                break;
            
            if(map->key_compare(map->table[index].key, key) == 0)
                return map->table + index;
            
            break;
        case MAP_DATA_STATE_EMPTY:
            /* 'key' does not exist within the table */
            return NULL;
        default:
            break;
        }
        
        index  += offset;
        offset += 2;
        
        index &= (map->capacity - 1);
    }
    
    return NULL;
}

struct map *map_new(const struct map_config *__restrict conf)
{
    struct map *map;
    int err;
    
    map = malloc(sizeof(*map));
    if (!map)
        return NULL;
    
    err = map_init(map, conf);
    if (err < 0) {
        free(map);
        errno = -err;
        return NULL;
    }
    
    return map;
}

void map_delete(struct map *__restrict map)
{
    map_destroy(map);
    free(map);
}

int map_init(struct map *__restrict map,
             const struct map_config *__restrict conf)
{
    unsigned int size = adjust(conf->size << 1, MAP_DEFAULT_SIZE);
    
    map->table = calloc(size, sizeof(*map->table));
    if (!map->table)
        return -errno;
    
    map->size     = 0;
    map->capacity = size;
    
    map->lower_bound = conf->lower_bound;
    map->upper_bound = conf->upper_bound;
    
    map->static_size = conf->static_size;
    
    map->key_compare = conf->key_compare;
    map->key_hash    = conf->key_hash;
    map->data_delete = conf->data_delete;
    
    return 0;
}

void map_destroy(struct map *__restrict map)
{
    map_clear(map);
    free(map->table);
}

void map_clear(struct map *__restrict map)
{
    int i;
    
    map->size = 0;
    
    if (!map->data_delete) {
        memset(map->table, 0, sizeof(*map->table) * map->capacity);
        return;
    }
    
    for (i = 0; i < map->capacity; ++i) {
        if (map->table[i].state == MAP_DATA_STATE_AVAILABLE) {
            map->data_delete(map->table[i].data);
            
            map->table[i].state = MAP_DATA_STATE_EMPTY;
        }
    }
}

int map_rehash(struct map *__restrict map, unsigned int size)
{
    /* ensure that we don't have to rehash for 'size' additional insertions */
    size = adjust((size + map->size) << 1, MAP_DEFAULT_SIZE);
    
    if (size == map->capacity)
        return 0;
    
    return map_resize(map, size);
}

/*
 * Note:
 * If there is a bug in this function it is very likely that
 * _map_lookup() suffers from the same bug.
 */
int map_insert(struct map *__restrict map, const void *key, void *data)
{
    unsigned int hash, index, offset, capacity;
    int err;
    
    if (!map->static_size && map_should_grow(map)) {
        capacity = map->capacity << 1;
        do {
            err = map_resize(map, capacity);
            capacity <<= 1;
        } while (err == -EBADSLT);
    }
    
    hash = map->key_hash(key);
    
    index = hash & (map->capacity - 1);
    offset = 1;
    
    while (offset < map->capacity) {
        if (map->table[index].state != MAP_DATA_STATE_AVAILABLE) {
            map->table[index].key   = key;
            map->table[index].data  = data;
            map->table[index].hash  = hash;
            map->table[index].state = MAP_DATA_STATE_AVAILABLE;
            
            map->size += 1;
            
            return 0;
        }

        index  += offset;
        offset += 2;
        
        index &= (map->capacity - 1);
    }
    
    return -EBADSLT;
}

void *map_retrieve(struct map *__restrict map, const void *key)
{
    struct entry *entry;
    
    entry = map_lookup(map, key);
    
    return (entry) ? entry->data : NULL;
}

void *map_take(struct map *__restrict map, const void *key)
{
    struct entry *entry;
    void *data;
    
    entry = map_lookup(map, key);
    if(!entry)
        return NULL;

    data = entry->data;

    entry->state = MAP_DATA_STATE_REMOVED;
    
    map->size -= 1;
    
    if(!map->static_size && map_should_shrink(map))
        map_resize(map, map->capacity >> 2);
    
    return data;
}

bool map_contains(const struct map *__restrict map, const void *key)
{
    return map_lookup(map, key) != NULL;
}

inline unsigned int map_size(const struct map *__restrict map)
{
    return map->size;
}

inline bool map_empty(const struct map *__restrict map)
{
    return map->size == 0;
}

void map_set_static_size(struct map *__restrict map, bool static_size)
{
    map->static_size = static_size;
}

bool map_static_size(const struct map *__restrict map)
{
    return map->static_size;
}

inline const void *entry_key(struct entry *__restrict e)
{
    return e->key;
}

inline void *entry_data(struct entry *__restrict e)
{
    return e->data;
}