#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "map.h"
#include "container_p.h"
#include "macro.h"

#define MAP_DEFAULT_CAPACITY 32

#define MAP_UPPER_TABLE_BOUND 60
#define MAP_LOWER_TABLE_BOUND 10

#define _map_should_grow(map)                                                  \
    (100 * (map)->size / (map)->capacity) >= MAP_UPPER_TABLE_BOUND
    
#define _map_should_shrink(map)                                                \
    (100 * (map)->size / (map)->capacity) < MAP_LOWER_TABLE_BOUND


static int _map_rehash(struct map *__restrict map, unsigned int capacity)
{
    struct map_entry *old_table;
    unsigned int i, old_capacity, old_entries;
    int err;
    
    old_entries  = map->size;
    old_capacity = map->capacity;
    old_table    = map->table;
    
    map->size     = 0;
    map->capacity = max(capacity, MAP_DEFAULT_CAPACITY);
    map->table    = calloc(map->capacity, sizeof(*map->table));

    if(!map->table)
        return -errno;
    
    for(i = 0; i < old_capacity; ++i) {
        if(old_table[i].state == MAP_DATA_STATE_AVAILABLE) {

            err = map_insert(map, old_table[i].key, old_table[i].data);
            if(err < 0) {
                /* revert to old table, which wasn't changed */
                free(map->table);
                
                map->size     = old_entries;
                map->capacity = old_capacity;
                map->table    = old_table;
                
                return err;
            }
        }
    }
    
    free(old_table);
    
    return 0;
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
static struct map_entry *_map_lookup(const struct map *__restrict map,
                                     const void *__restrict key)
{
    unsigned int hash, index, offset;
    
    hash = map->key_hash(key);

    index = hash & (map->capacity - 1);
    offset = 1;
    
    while(offset < map->capacity) {

        switch(map->table[index].state) {
            case MAP_DATA_STATE_AVAILABLE:
                if(map->table[index].hash != hash )
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

struct map *map_new(unsigned int size,
                    int (*key_compare)(const void *, const void *),
                    unsigned int (*key_hash)(const void *))
{
    struct map *map;
    int err;
    
    map = malloc(sizeof(*map));
    if(!map)
        return NULL;
    
    err = map_init(map, size, key_compare, key_hash);
    if(err < 0) {
        free(map);
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
             unsigned int size,
             int (*key_compare)(const void *, const void *),
             unsigned int (*key_hash)(const void *))
{
    size = adjust(size << 1, MAP_DEFAULT_CAPACITY);
    
    map->table = calloc(size, sizeof(*map->table));
    if(!map->table)
        return -errno;
    
    map->capacity = size;
    map->size = 0;
    
    map->key_compare = key_compare;
    map->key_hash    = key_hash;
    map->data_delete = NULL;
    
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
    
    if(!map->data_delete) {
        memset(map->table, 0, sizeof(*map->table) * map->capacity);
        return;
    }
    
    for(i = 0; i < map->capacity; ++i) {
        if(map->table[i].state == MAP_DATA_STATE_AVAILABLE) {
            map->data_delete(map->table[i].data);
            
            map->table[i].state = MAP_DATA_STATE_EMPTY;
        }
    }
}

int map_rehash(struct map *__restrict map, unsigned int size)
{
    /* ensure that we don't have to rehash for 'size' additional insertions */
    size = adjust((size + map->size) << 1, MAP_DEFAULT_CAPACITY);
    
    if(size == map->capacity)
        return 0;
    
    return _map_rehash(map, size);
}

/*
 * Note:
 * If there is a bug in this function it is very likely that
 * _map_lookup() suffers from the same bug.
 */
int map_insert(struct map *__restrict map, const void *key, void *data)
{
    unsigned int hash, index, offset;
    
    if(_map_should_grow(map))
        _map_rehash(map, map->capacity << 1);
    
    hash = map->key_hash(key);
    
    index = hash & (map->capacity - 1);
    offset = 1;
    
    while(offset < map->capacity) {
        if(map->table[index].state != MAP_DATA_STATE_AVAILABLE) {
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
    
    return -1;
}

void *map_retrieve(struct map *__restrict map, const void *key)
{
    struct map_entry *entry;
    
    entry = _map_lookup(map, key);
    
    return (entry) ? entry->data : NULL;
}

void *map_take(struct map *__restrict map, const void *key)
{
    struct map_entry *entry;
    void *data;
    
    entry = _map_lookup(map, key);
    if(!entry)
        return NULL;

    data = entry->data;

    entry->state = MAP_DATA_STATE_REMOVED;
    
    map->size -= 1;
    
    if(_map_should_shrink(map))
        _map_rehash(map, map->capacity >> 2);
    
    return data;
}

bool map_contains(const struct map *__restrict map, const void *key)
{
    return _map_lookup(map, key) != NULL;
}

inline unsigned int map_size(const struct map *__restrict map)
{
    return map->size;
}

inline bool map_empty(const struct map *__restrict map)
{
    return map->size == 0;
}

inline void map_set_key_compare(struct map *__restrict map,
                                int (*key_compare)(const void *, const void *))
{
    map->key_compare = key_compare;
}

inline void map_set_key_hash(struct map *__restrict map, 
                              unsigned int (*key_hash)(const void *))
{
    map->key_hash = key_hash;
}

inline void map_set_data_delete(struct map *__restrict map,
                                void (*data_delete)(void *))
{
    map->data_delete = data_delete;
}

int (*map_key_compare(struct map *__restrict map))(const void *, const void *)
{
    return map->key_compare;
}

unsigned int (*map_key_hash(struct map *__restrict map))(const void *)
{
    return map->key_hash;
}

void (*map_data_delete(struct map *__restrict map))(void *)
{
    return map->data_delete;
}