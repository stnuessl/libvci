#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "item.h"
#include "list.h"
#include "hash.h"

#define HASH_DEFAULT_START_SIZE 10
#define HASH_DEFAUT_MIN_SIZE    10

#define HASH_UPPER_FILLING_EDGE 75
#define HASH_LOWER_FILLING_EDGE 25


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

inline static int _hash_key(const struct hash *__restrict hash, 
                            const void *__restrict key)
{
    uint64_t hval;
    size_t key_size;
    
    key_size = (hash->_key_length) ? hash->_key_length(key) : hash->_key_size;
    
    if(hash->_key_hash)
        hval = hash->_key_hash(key, key_size);
    else
        hval = _default_key_hash(key, key_size);
    
    return hval % hash->_num_lists;
}

inline static void _hash_adjust_size(struct hash *__restrict hash, int adj)
{
    hash->_size += adj;
}

static void _hash_insert(struct hash *__restrict hash, 
                         struct item *__restrict item)
{
    int index;

    index = _hash_key(hash, item_key(item));

    list_insert_item_front(hash->_lists + index, item);
}


inline static int _hash_occupation(const struct hash *__restrict hash)
{
    return 100 * hash->_size / hash->_num_lists;
}

inline static bool _hash_should_grow(const struct hash *__restrict hash)
{
    return _hash_occupation(hash) > HASH_UPPER_FILLING_EDGE;
}

inline static bool _hash_should_shrink(const struct hash *__restrict hash)
{
    return _hash_occupation(hash) < HASH_LOWER_FILLING_EDGE;
}

inline static int _hash_get_best_size(const struct hash *__restrict hash)
{
    /* new occupation will be 50% */
    return (hash->_size > 4) ?  hash->_size << 1 : HASH_DEFAUT_MIN_SIZE;
}

struct hash *hash_new(int size, size_t key_size)
{
    struct hash *hash;
    
    hash = malloc(sizeof(*hash));
    if(!hash)
        return NULL;
    
    if(hash_init(hash, size, key_size) < 0) {
        free(hash);
        return NULL;
    }
    
    return hash;
}

void hash_delete(struct hash *__restrict hash)
{
    hash_destroy(hash);
    free(hash);
}


int hash_init(struct hash *__restrict hash, int size, size_t key_size)
{
    if(size <= 0)
        size = HASH_DEFAULT_START_SIZE;
    
    memset(hash, 0, sizeof(*hash));
    
    hash->_lists = calloc(size, sizeof(*hash->_lists));
    if(!hash->_lists)
        return -errno;
    
    hash->_num_lists = size;
    hash->_key_size  = key_size;
    
    /* lists are already initialized: calloc() zeroes the returned memory */

    return 0;
}

void hash_destroy(struct hash *__restrict hash)
{
    int i;
    void (*data_delete)(void *);
    void (*key_delete)(void *);
    
    data_delete = hash->_data_delete;
    key_delete  = hash->_key_delete;
    
    for(i = 0; i < hash->_num_lists; ++i)
        list_destroy(hash->_lists + i, data_delete, key_delete);
    
    free(hash->_lists);
}

int hash_insert(struct hash *__restrict hash, 
                void *__restrict data, 
                void *__restrict key)
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    hash_insert_item(hash, item);
    
    return 0;
}

void hash_insert_item(struct hash *__restrict hash, 
                      struct item *__restrict item)
{
    if(_hash_should_grow(hash))
        hash_resize(hash, 0);
    
    _hash_insert(hash, item);
    
    _hash_adjust_size(hash, 1);
}

void *hash_take(struct hash *__restrict hash, const void *__restrict key)
{
    struct item *item;
    void *data;
    
    item = hash_take_item(hash, key);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, hash->_key_delete);
    
    return data;
}

struct item *hash_take_item(struct hash *__restrict hash, 
                            const void *__restrict key)
{
    struct item *item;
    int index;

    index = _hash_key(hash, key);

    item = list_take_item(hash->_lists + index, key, hash->_key_compare);
    if(!item)
        return NULL;
    
    _hash_adjust_size(hash, -1);
    
    if(_hash_should_shrink(hash))
        hash_resize(hash, 0);
    
    return item;
}

void *hash_retrieve(struct hash *__restrict hash, const void *__restrict key)
{
    struct item *item;
    
    item = hash_retrieve_item(hash, key);
    
    return (item) ? item_data(item) : NULL;
}

struct item *hash_retrieve_item(struct hash *__restrict hash, 
                                const void *__restrict key)
{
    int index;
    
    index = _hash_key(hash, key);
    
    return list_retrieve_item(hash->_lists + index, key, hash->_key_compare);
}

inline int hash_size(const struct hash *__restrict hash)
{
    return hash->_size;
}

void hash_delete_item(struct hash *__restrict hash, 
                      const void *__restrict key)
{
    struct item *item;
    
    item = hash_take_item(hash, key);
    
    item_delete(item, hash->_data_delete, hash->_key_delete);
}

int hash_resize(struct hash *__restrict hash, int size)
{
    struct list *old_lists;
    int old_size;
    
    if(size <= 0)
        size = _hash_get_best_size(hash);
    
    old_lists = hash->_lists;
    old_size  = hash->_num_lists;
    
    hash->_lists = calloc(size, sizeof(*hash->_lists));
    if(!hash->_lists) {
        hash->_lists = old_lists;
        return -errno;
    }
    
    hash->_num_lists = size;
    
    while(old_size--) {
        while(!list_empty(old_lists + old_size))
            _hash_insert(hash, list_take_item_front(old_lists + old_size));
    }
    /* delete list object, list is empty anyway */
    free(old_lists);
    
    return 0;
}

bool hash_contains(struct hash *__restrict hash, const void *__restrict key)
{
    return hash_retrieve_item(hash, key) != NULL;
}

inline bool hash_empty(const struct hash *__restrict hash)
{
    return hash_size(hash) == 0;
}

struct item *hash_begin(struct hash *__restrict hash)
{
    int i;
    
    for(i = 0; i < hash->_num_lists; ++i) {
        if(!list_empty(hash->_lists + i))
            return list_begin(hash->_lists + i);
    }
    
    return NULL;
}

struct item *hash_end(struct hash *__restrict hash)
{
    int i; 
    
    for(i = hash->_num_lists - 1; i >= 0; --i) {
        if(!list_empty(hash->_lists + i))
            return list_end(hash->_lists + i);
    }
    
    return NULL;
}

struct item *hash_item_prev(struct hash *__restrict hash, 
                            struct item *__restrict item)
{
    int index;
    
    if(!hash_contains(hash, item_key(item)))
        return NULL;
    
    if(item_prev(item))
        return item_prev(item);
    
    index = _hash_key(hash, item_key(item));
    
    while(index--) {
        if(!list_empty(hash->_lists + index))
            return list_end(hash->_lists + index);
    }
    
    return NULL;
}

struct item *hash_item_next(struct hash *__restrict hash,
                            struct item *__restrict item)
{
    int index;
    
    if(!hash_contains(hash, item_key(item)))
        return NULL;
    
    if(item_next(item))
        return item_next(item);
    
    index = _hash_key(hash, item_key(item));
    
    while(++index < hash->_num_lists) {
        if(!list_empty(hash->_lists + index))
            return list_begin(hash->_lists + index);
    }
    
    return NULL;
}


#define HASH_DEFINE_SETGET(name, type)                                         \
                                                                               \
inline void hash_set_##name(struct hash *__restrict hash, type name)           \
{                                                                              \
    hash->_##name = name;                                                      \
}                                                                              \
                                                                               \
inline type hash_##name(const struct hash *__restrict hash)                    \
{                                                                              \
    return hash->_##name;                                                      \
}

HASH_DEFINE_SETGET(key_size, size_t)

#undef HASH_DEFINE_SETGET

#define HASH_DEFINE_SET_CALLBACK(name, type, param)                            \
                                                                               \
inline void hash_set_##name(struct hash *__restrict hash, type (*name)param)   \
{                                                                              \
    hash->_##name = name;                                                      \
}


HASH_DEFINE_SET_CALLBACK(key_hash, uint64_t, (const void *, size_t))
HASH_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
HASH_DEFINE_SET_CALLBACK(key_length, size_t, (const void *))
HASH_DEFINE_SET_CALLBACK(key_delete, void, (void *))
HASH_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef HASH_DEFINE_SET_CALLBACK
