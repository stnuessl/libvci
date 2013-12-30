#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>

#include "link.h"
#include "list.h"
#include "cache.h"
#include "macros.h"


static struct cache_entry *_cache_lookup(struct cache *__restrict cache, 
                                         const void *key)
{
    unsigned int hash, index, i;
    
    hash = cache->key_hash(key);
    
    index = hash % cache->capacity;
    
    for(i = 0; i < cache->capacity; ++i) {
        switch(cache->table[index].state) {
            case DATA_AVAILABLE:
                if(cache->table[index].hash != hash)
                    break;
                
                if(cache->key_compare(cache->table[index].key, key) == 0)
                    return cache->table + index;
                
                break;
            case DATA_EMPTY:
                /* 'key' does not exist within the table */
                return NULL;
            default:
                break;
        }
        
        index += 1;
        
        if(index >= cache->capacity)
            index -= cache->capacity;
    }
    
    return NULL;
}


struct cache *cache_new(unsigned int capacity,
                        int (*key_compare)(const void *, const void *),
                        unsigned int (*key_hash)(const void *))
{
    struct cache *cache;
    int err;
    
    cache = malloc(sizeof(*cache));
    if(!cache)
        return NULL;
    
    err = cache_init(cache, capacity, key_compare, key_hash);
    if(err < 0) {
        free(cache);
        return NULL;
    }
    
    return cache;
}

void cache_delete(struct cache *__restrict cache)
{
    cache_destroy(cache);
    free(cache);
}

int cache_init(struct cache *__restrict cache,
               unsigned int capacity,
               int (*key_compare)(const void *, const void *),
               unsigned int (*key_hash)(const void *))
{
    cache->max_size = capacity;
    
    /* table will be filled to a maximum of 25 % */
    capacity = capacity << 2;
    
    cache->table = calloc(capacity, sizeof(*cache->table));
    if(!cache->table)
        return -errno;
        
    list_init(&cache->list);
    
    cache->capacity    = capacity;
    cache->size        = 0;
    cache->key_compare = key_compare;
    cache->key_hash    = key_hash;
    cache->data_delete = NULL;
    
    return 0;
}

void cache_destroy(struct cache *__restrict cache)
{
    cache_clear(cache);
    free(cache->table);
}

void cache_clear(struct cache *__restrict cache)
{
    unsigned int i;

    if(!cache->data_delete) {
        memset(cache->table, 0, cache->capacity * sizeof(*cache->table));
        return;
    }
    
    for(i = 0; i < cache->capacity; ++i) {
        if(cache->table[i].data != NULL) {
            cache->data_delete(cache->table[i].data);
            
            cache->table[i].data = NULL;
            cache->table[i].key  = NULL;
        }
    }
}

void cache_insert(struct cache *__restrict cache, const void *key, void *data)
{
    struct link *link;
    struct cache_entry *entry;
    unsigned int index, hash;
    
    if(cache->size >= cache->max_size) {
        link = list_take_back(&cache->list);
        entry = container_of(link, struct cache_entry, link);
        
        entry->hash  = 0;
        entry->key   = NULL;
        entry->state = DATA_REMOVED;
        
        if(cache->data_delete)
            cache->data_delete(entry->data);
        
        entry->data = NULL;
    } else {
        cache->size += 1;
    }
    
    hash = cache->key_hash(key);
    
    index = hash % cache->capacity;
    
    while(1) {
        if(cache->table[index].state != DATA_AVAILABLE) {
            cache->table[index].hash  = hash;
            cache->table[index].key   = key;
            cache->table[index].data  = data;
            cache->table[index].state = DATA_AVAILABLE;
            
            list_insert_front(&cache->list, &cache->table[index].link);
            
            return;
        }
        
        index += 1;
        
        if(index >= cache->capacity)
            index -= cache->capacity;
    }
}

void *cache_retrieve(struct cache *__restrict cache, const void *key)
{
    struct cache_entry *entry;
    
    entry = _cache_lookup(cache, key);
    if(!entry)
        return NULL;
    
    list_take(&entry->link);
    list_insert_front(&cache->list, &entry->link);
    
    return entry->data;
}

void* cache_take(struct cache *__restrict cache, const void* key)
{
    struct cache_entry *entry;
    
    entry = _cache_lookup(cache, key);
    
    if(!entry)
        return NULL;
    
    list_take(&entry->link);
    
    entry->state = DATA_REMOVED;
    cache->size -= 1;
    
    return entry->data;
}

inline bool cache_empty(const struct cache *__restrict cache)
{
    return cache->size == 0;
}

inline bool cache_full(const struct cache *__restrict cache)
{
    return cache->size >= cache->max_size;
}

inline unsigned int cache_size(const struct cache *__restrict cache)
{
    return cache->size;
}

inline unsigned int cache_capacity(const struct cache *__restrict cache)
{
    return cache->capacity;
}

inline void cache_set_data_delete(struct cache *__restrict cache, void (*data_delete)(void *))
{
    cache->data_delete = data_delete;
}

unsigned int (*cache_key_hash(struct cache *__restrict cache))(const void *)
{
    return cache->key_hash;
}

int (*cache_key_compare(struct cache *__restrict cache))
                       (const void *, const void *)
{
    return cache->key_compare;
}

void (*cache_data_delete(struct cache *__restrict cache))(void *)
{
    return cache->data_delete;
}