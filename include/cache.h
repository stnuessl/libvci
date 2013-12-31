#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdbool.h>

#include "link.h"

enum cache_data_state {
    DATA_EMPTY          = 0x00,
    DATA_AVAILABLE      = 0x01,
    DATA_REMOVED        = 0x02
};

struct cache_entry {
    unsigned int hash;
    const void *key;
    void *data;
    
    enum cache_data_state state;
    struct link link;
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

struct cache *cache_new(unsigned int capacity,
                        int (*key_compare)(const void *, const void *),
                        unsigned int (*key_hash)(const void *));

void cache_delete(struct cache *__restrict cache);

int cache_init(struct cache *__restrict cache,
               unsigned int capacity,
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