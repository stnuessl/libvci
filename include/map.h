#ifndef _MAP_H_
#define _MAP_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "item.h"
#include "list.h"

struct map {
    struct list *_table;
    
    int _size;
    int _num_lists;
    
    uint64_t (*_key_hash)(const void *, size_t);
    int (*_key_compare)(const void *, const void *);
    size_t (*_key_length)(const void *);
    void (*_key_delete)(void *);
    void (*_data_delete)(void *);
    
    size_t _key_size;
};

struct map *map_new(int size, size_t key_size);

void map_delete(struct map *__restrict map);

int map_init(struct map *__restrict map, int size, size_t key_size);

void map_destroy(struct map *__restrict map);

int map_insert(struct map *__restrict map, 
               void *__restrict data, 
               void *__restrict key);

void map_insert_item(struct map *__restrict map, 
                     struct item *__restrict item);

void *map_take(struct map *__restrict map, const void *__restrict key);

struct item *map_take_item(struct map *__restrict map, 
                           const void *__restrict key);

void *map_retrieve(struct map *__restrict map, const void *__restrict key);

struct item *map_retrieve_item(struct map *__restrict map, 
                                const void *__restrict key);

void map_delete_item(struct map *__restrict map, 
                      const void *__restrict key);

inline int map_size(const struct map *__restrict map);

int map_resize(struct map *__restrict map, int size);

bool map_contains(struct map *__restrict map, const void *__restrict key);

inline bool map_empty(const struct map *__restrict map);

struct item *map_begin(struct map *__restrict map);

struct item *map_end(struct map *__restrict map);

struct item *map_item_prev(struct map *__restrict map, 
                            struct item *__restrict item);

struct item *map_item_next(struct map *__restrict map,
                           struct item *__restrict item);


#define MAP_DEFINE_SETGET(name, type)                                          \
                                                                               \
inline void map_set_##name(struct map *__restrict map, type name);             \
                                                                               \
inline type map_##name(const struct map *__restrict map);

MAP_DEFINE_SETGET(key_size, size_t)

#undef MAP_DEFINE_SETGET


#define MAP_DEFINE_SET_CALLBACK(name, type, param)                             \
                                                                               \
inline void map_set_##name(struct map *__restrict map, type (*name)param);

MAP_DEFINE_SET_CALLBACK(key_hash, uint64_t, (const void *, size_t))
MAP_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
MAP_DEFINE_SET_CALLBACK(key_length, size_t, (const void *))
MAP_DEFINE_SET_CALLBACK(key_delete, void, (void *))
MAP_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef MAP_DEFINE_SET_CALLBACK


#define map_for_each(map, item)                                                \
for((item) = map_begin((map));                                                 \
    (item);                                                                    \
    (item) = map_item_next((map), (item)))

#endif /* _MAP_H_ */