#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "item.h"
#include "list.h"
#include "map.h"

#define MAP_DEFAULT_START_SIZE 10
#define MAP_DEFAUT_MIN_SIZE    10

#define MAP_UPPER_FILLING_EDGE 75
#define MAP_LOWER_FILLING_EDGE 25


inline static uint64_t _default_key_hash(const void *data, size_t size)
{
    const unsigned char *buf;
    unsigned long hval;

    buf  = data;
    hval = 1;
    
    while(size--) {
        hval += *buf++;
        hval += (hval << 10);
        hval ^= (hval >> 6);
        hval &= 0x0fffffff;
    }
    
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    return hval;
}

inline static int _map_hash_key(const struct map *__restrict map, 
                                const void *__restrict key)
{
    uint64_t hval;
    size_t key_size;
    
    key_size = (map->_key_length) ? map->_key_length(key) : map->_key_size;
    
    if(map->_key_hash)
        hval = map->_key_hash(key, key_size);
    else
        hval = _default_key_hash(key, key_size);
    
    return hval % map->_num_lists;
}

inline static void _map_adjust_size(struct map *__restrict map, int adj)
{
    map->_size += adj;
}

static void _map_insert(struct map *__restrict map, 
                        struct item *__restrict item)
{
    int index;

    index = _map_hash_key(map, item_key(item));

    list_insert_item_front(map->_table + index, item);
}


inline static int _map_occupation(const struct map *__restrict map)
{
    return 100 * map->_size / map->_num_lists;
}

inline static bool _map_should_grow(const struct map *__restrict map)
{
    return _map_occupation(map) > MAP_UPPER_FILLING_EDGE;
}

inline static bool _map_should_shrink(const struct map *__restrict map)
{
    return _map_occupation(map) < MAP_LOWER_FILLING_EDGE;
}

inline static int _map_get_best_size(const struct map *__restrict map)
{
    /* new occupation will be 50% */
    return (map->_size > 4) ?  map->_size << 1 : MAP_DEFAUT_MIN_SIZE;
}

struct map *map_new(int size, size_t key_size)
{
    struct map *map;
    
    map = malloc(sizeof(*map));
    if(!map)
        return NULL;
    
    if(map_init(map, size, key_size) < 0) {
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


int map_init(struct map *__restrict map, int size, size_t key_size)
{
    if(size <= 0)
        size = MAP_DEFAULT_START_SIZE;
    
    memset(map, 0, sizeof(*map));
    
    map->_table = calloc(size, sizeof(*map->_table));
    if(!map->_table)
        return -errno;
    
    map->_num_lists = size;
    map->_key_size  = key_size;
    
    /* lists are already initialized: calloc() zeroes the returned memory */

    return 0;
}

void map_destroy(struct map *__restrict map)
{
    int i;
    void (*data_delete)(void *);
    void (*key_delete)(void *);
    
    data_delete = map->_data_delete;
    key_delete  = map->_key_delete;
    
    for(i = 0; i < map->_num_lists; ++i)
        list_destroy(map->_table + i, data_delete, key_delete);
    
    free(map->_table);
}

int map_insert(struct map *__restrict map, 
               void *__restrict data, 
               void *__restrict key)
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    map_insert_item(map, item);
    
    return 0;
}

void map_insert_item(struct map *__restrict map, 
                     struct item *__restrict item)
{
    if(_map_should_grow(map))
        map_resize(map, 0);
    
    _map_insert(map, item);
    
    _map_adjust_size(map, 1);
}

void *map_take(struct map *__restrict map, const void *__restrict key)
{
    struct item *item;
    void *data;
    
    item = map_take_item(map, key);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, map->_key_delete);
    
    return data;
}

struct item *map_take_item(struct map *__restrict map, 
                           const void *__restrict key)
{
    struct item *item;
    int index;

    index = _map_hash_key(map, key);

    item = list_take_item(map->_table + index, key, map->_key_compare);
    if(!item)
        return NULL;
    
    _map_adjust_size(map, -1);
    
    if(_map_should_shrink(map))
        map_resize(map, 0);
    
    return item;
}

void *map_retrieve(struct map *__restrict map, const void *__restrict key)
{
    struct item *item;
    
    item = map_retrieve_item(map, key);
    
    return (item) ? item_data(item) : NULL;
}

struct item *map_retrieve_item(struct map *__restrict map, 
                               const void *__restrict key)
{
    int index;
    
    index = _map_hash_key(map, key);
    
    return list_retrieve_item(map->_table + index, key, map->_key_compare);
}

inline int map_size(const struct map *__restrict map)
{
    return map->_size;
}

void map_delete_item(struct map *__restrict map, 
                     const void *__restrict key)
{
    struct item *item;
    
    item = map_take_item(map, key);
    
    item_delete(item, map->_data_delete, map->_key_delete);
}

int map_resize(struct map *__restrict map, int size)
{
    struct list *old_lists;
    int old_size;
    
    if(size <= 0)
        size = _map_get_best_size(map);
    
    old_lists = map->_table;
    old_size  = map->_num_lists;
    
    map->_table = calloc(size, sizeof(*map->_table));
    if(!map->_table) {
        map->_table = old_lists;
        return -errno;
    }
    
    map->_num_lists = size;
    
    while(old_size--) {
        while(!list_empty(old_lists + old_size))
            _map_insert(map, list_take_item_front(old_lists + old_size));
    }
    /* delete list object, list is empty anyway */
    free(old_lists);
    
    return 0;
}

bool map_contains(struct map *__restrict map, const void *__restrict key)
{
    return map_retrieve_item(map, key) != NULL;
}

inline bool map_empty(const struct map *__restrict map)
{
    return map_size(map) == 0;
}

struct item *map_begin(struct map *__restrict map)
{
    int i;
    
    for(i = 0; i < map->_num_lists; ++i) {
        if(!list_empty(map->_table + i))
            return list_begin(map->_table + i);
    }
    
    return NULL;
}

struct item *map_end(struct map *__restrict map)
{
    int i; 
    
    for(i = map->_num_lists - 1; i >= 0; --i) {
        if(!list_empty(map->_table + i))
            return list_end(map->_table + i);
    }
    
    return NULL;
}

struct item *map_item_prev(struct map *__restrict map, 
                           struct item *__restrict item)
{
    int index;
    
    if(!map_contains(map, item_key(item)))
        return NULL;
    
    if(item_prev(item))
        return item_prev(item);
    
    index = _map_hash_key(map, item_key(item));
    
    while(index--) {
        if(!list_empty(map->_table + index))
            return list_end(map->_table + index);
    }
    
    return NULL;
}

struct item *map_item_next(struct map *__restrict map,
                           struct item *__restrict item)
{
    int index;
    
    if(!map_contains(map, item_key(item)))
        return NULL;
    
    if(item_next(item))
        return item_next(item);
    
    index = _map_hash_key(map, item_key(item));
    
    while(++index < map->_num_lists) {
        if(!list_empty(map->_table + index))
            return list_begin(map->_table + index);
    }
    
    return NULL;
}


#define MAP_DEFINE_SETGET(name, type)                                          \
                                                                               \
inline void map_set_##name(struct map *__restrict map, type name)              \
{                                                                              \
    map->_##name = name;                                                       \
}                                                                              \
                                                                               \
inline type map_##name(const struct map *__restrict map)                       \
{                                                                              \
    return map->_##name;                                                       \
}

MAP_DEFINE_SETGET(key_size, size_t)

#undef MAP_DEFINE_SETGET

#define MAP_DEFINE_SET_CALLBACK(name, type, param)                             \
                                                                               \
inline void map_set_##name(struct map *__restrict map, type (*name)param)     \
{                                                                              \
    map->_##name = name;                                                       \
}


MAP_DEFINE_SET_CALLBACK(key_hash, uint64_t, (const void *, size_t))
MAP_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
MAP_DEFINE_SET_CALLBACK(key_length, size_t, (const void *))
MAP_DEFINE_SET_CALLBACK(key_delete, void, (void *))
MAP_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef MAP_DEFINE_SET_CALLBACK

