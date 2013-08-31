#ifndef _HASH_H_
#define _HASH_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "item.h"
#include "list.h"

struct hash {
    struct list *_lists;
    
    int _size;
    int _num_lists;
    
    uint64_t (*_key_hash)(const void *, size_t);
    int (*_key_compare)(const void *, const void *);
    size_t (*_key_length)(const void *);
    void (*_key_delete)(void *);
    void (*_data_delete)(void *);
    
    size_t _key_size;
};

struct hash *hash_new(int size, size_t key_size);

void hash_delete(struct hash *__restrict hash);

int hash_init(struct hash *__restrict hash, int size, size_t key_size);

void hash_destroy(struct hash *__restrict hash);

int hash_insert(struct hash *__restrict hash, 
                void *__restrict data, 
                void *__restrict key);

void hash_insert_item(struct hash *__restrict hash, 
                      struct item *__restrict item);

void *hash_take(struct hash *__restrict hash, const void *__restrict key);

struct item *hash_take_item(struct hash *__restrict hash, 
                            const void *__restrict key);

void *hash_retrieve(struct hash *__restrict hash, const void *__restrict key);

struct item *hash_retrieve_item(struct hash *__restrict hash, 
                                const void *__restrict key);

void hash_delete_item(struct hash *__restrict hash, 
                      const void *__restrict key);

inline int hash_size(const struct hash *__restrict hash);

int hash_resize(struct hash *__restrict hash, int size);

bool hash_contains(struct hash *__restrict hash, const void *__restrict key);

inline bool hash_empty(const struct hash *__restrict hash);

struct item *hash_begin(struct hash *__restrict hash);

struct item *hash_end(struct hash *__restrict hash);

struct item *hash_item_prev(struct hash *__restrict hash, 
                            struct item *__restrict item);

struct item *hash_item_next(struct hash *__restrict hash,
                            struct item *__restrict item);


#define HASH_DEFINE_SETGET(name, type)                                         \
                                                                               \
inline void hash_set_##name(struct hash *__restrict hash, type name);          \
                                                                               \
inline type hash_##name(const struct hash *__restrict hash);

HASH_DEFINE_SETGET(key_size, size_t)

#undef HASH_DEFINE_SETGET


#define HASH_DEFINE_SET_CALLBACK(name, type, param)                            \
                                                                               \
inline void hash_set_##name(struct hash *__restrict hash, type (*name)param);

HASH_DEFINE_SET_CALLBACK(key_hash, uint64_t, (const void *, size_t))
HASH_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
HASH_DEFINE_SET_CALLBACK(key_length, size_t, (const void *))
HASH_DEFINE_SET_CALLBACK(key_delete, void, (void *))
HASH_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef HASH_DEFINE_SET_CALLBACK


#define hash_for_each(hash, item)                                              \
for((item) = hash_begin((hash));                                               \
    (item);                                                                    \
    (item) = hash_item_next((hash), (item)))

#endif /* _HASH_H_ */