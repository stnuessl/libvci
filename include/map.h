#ifndef _MAP_H_
#define _MAP_H_

#include <stdbool.h>

enum map_data_state {
    MAP_DATA_STATE_EMPTY        = 0x00,
    MAP_DATA_STATE_AVAILABLE    = 0x01,
    MAP_DATA_STATE_REMOVED      = 0x02
};

struct map_entry {
    const void *key;
    void *data;
    
    unsigned int hash;
    enum map_data_state state;
};

struct map {
    struct map_entry *table;
    unsigned int size;
    unsigned int capacity;

    int (*key_compare)(const void *, const void *);
    unsigned int (*key_hash)(const void *);
    void (*data_delete)(void *);
};

struct map *map_new(unsigned int size,
                    int (*key_compare)(const void *, const void *),
                    unsigned int (*key_hash)(const void *));

void map_delete(struct map *__restrict map);

int map_init(struct map *__restrict map,
             unsigned int size,
             int (*key_compare)(const void *, const void *),
             unsigned int (*key_hash)(const void *));

void map_destroy(struct map *__restrict map);

void map_clear(struct map *__restrict map);

int map_rehash(struct map *__restrict map, unsigned int size);

int map_insert(struct map *__restrict map, const void *key, void *data);

void *map_retrieve(struct map *__restrict map, const void *key);

void *map_take(struct map *__restrict map, const void *key);

bool map_contains(struct map *__restrict map, const void *key);

inline unsigned int map_size(const struct map *__restrict map);

inline bool map_empty(const struct map *__restrict map);

inline void map_set_key_compare(struct map *__restrict map,
                                int (*key_compare)(const void *, const void *));

inline void map_set_key_hash(struct map *__restrict map, 
                              unsigned int (*key_hash)(const void *));

inline void map_set_data_delete(struct map *__restrict map,
                                void (*data_delete)(void *));

int (*map_key_compare(struct map *__restrict map))(const void *, const void *);

unsigned int (*map_key_hash(struct map *__restrict map))(const void *);

void (*map_data_delete(struct map *__restrict map))(void *);


#endif /* _MAP_H_ */