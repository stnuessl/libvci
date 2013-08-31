#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "item.h"
#include "mempool.h"

#define ITEM_DEFAULT_NUM 1000

static struct mempool *_item_pool(void);

static void _item_pool_delete(void)
{
    mempool_delete(_item_pool());
}

static struct mempool *_item_pool(void)
{
    static struct mempool *pool;
    
    if(unlikely(!pool)) {
        pool = mempool_new(ITEM_DEFAULT_NUM, sizeof(struct item));
        if(!pool)
            return NULL;
        
        if(atexit(&_item_pool_delete) != 0) {
            mempool_delete(pool);
            return NULL;
        }
    }
    
    return pool;
}

struct item *item_new(void *__restrict data, void *__restrict key)
{
    struct mempool *pool;
    struct item *item;
    
    pool = _item_pool();
    if(unlikely(!pool))
        return NULL;
    
    item = mempool_alloc_chunk(pool);
    if(unlikely(!item))
        return NULL;
    
    memset(item, 0, sizeof(*item));
    
    item_set_data(item, data);
    item_set_key(item, key);
    
    return item;
}

struct item *item_clone(struct item *__restrict item, 
                        void *(*data_clone)(void *), 
                        void *(*key_clone)(void *),
                        void (*data_delete)(void *),
                        void (*key_delete)(void *))
{
    void *data;
    void *key;
    struct item *clone;
    
    data = data_clone(item_data(item));
    if(unlikely(!data))
        goto fail;
    
    key = key_clone(item_key(item));
    if(unlikely(!key))
        goto free_data;
    
    clone = item_new(data, key);
    if(unlikely(!clone))
        goto free_key;
    
    return clone;
    
free_key:
    key_delete(data);
free_data:
    data_delete(data);
fail:
    return NULL;
}

void item_delete(struct item *__restrict item,
                 void (*data_delete)(void *),
                 void (*key_delete)(void *))
{
    if(data_delete)
        data_delete(item_data(item));
    
    if(key_delete)
        key_delete(item_key(item));
    
    mempool_free_chunk(_item_pool(), item);
}

void item_swap(struct item *__restrict a, struct item *__restrict b)
{
    void *data, *key;

    data = item_data(a);
    item_set_data(a, item_data(b));
    item_set_data(b, data);
    
    key  = item_key(a);
    item_set_key(a, item_key(b));
    item_set_key(b, key);
}

struct item *item_nth_next(struct item *__restrict item, int nth)
{
    while(nth--)
        item = item_next(item);
    
    return item;
}

struct item *item_nth_prev(struct item *__restrict item, int nth)
{
    while(nth--)
        item = item_prev(item);
    
    return item;
}

#define ITEM_DEFINE_SETGET(name, type)                                         \
                                                                               \
inline void item_set_##name(struct item *__restrict item, type name)           \
{                                                                              \
    item->_##name = name;                                                      \
}                                                                              \
                                                                               \
inline type item_##name(struct item *__restrict item)                          \
{                                                                              \
    return item->_##name;                                                      \
}                                                                              \

ITEM_DEFINE_SETGET(key, void *)
ITEM_DEFINE_SETGET(data, void *)

#undef ITEM_DEFINE_SETGET

#define ITEM_DEFINE_SETGET_ITEMS(name, which)                                  \
                                                                               \
inline void item_set_##name(struct item *__restrict item, struct item *name)   \
{                                                                              \
    item->_items[which] = name;                                                \
}                                                                              \
                                                                               \
inline struct item *item_##name(struct item *__restrict item)                  \
{                                                                              \
    return item->_items[which];                                                \
}                                                                              \
                                                                               \
inline struct item **item_ref_##name(struct item *__restrict item)             \
{                                                                              \
    return item->_items + which;                                               \
}

ITEM_DEFINE_SETGET_ITEMS(prev, 0)
ITEM_DEFINE_SETGET_ITEMS(next, 1)
ITEM_DEFINE_SETGET_ITEMS(left, 0)
ITEM_DEFINE_SETGET_ITEMS(right, 1)

#undef ITEM_DEFINE_SETGET_ITEMS