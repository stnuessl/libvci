#include <pthread.h>
#include <errno.h>

#include "mempool.h"
#include "item.h"
#include "map.h"
#include "macros.h"
#include "item_allocator.h"

#define ITEM_ALLOCATOR_SIZE 1024

static pthread_mutex_t _mutex_pool;
static pthread_once_t _pool_initialized = PTHREAD_ONCE_INIT;
static struct mempool *_pool = NULL;

static void _item_allocator_init(void)
{
    if(!_pool)
        item_allocator_init(ITEM_ALLOCATOR_SIZE);
}

int item_allocator_init(int size)
{
    int err;

    _pool = mempool_new(size, sizeof(struct item));
    if(!_pool) {
        err = -errno;
        goto out;
    }
    
    err = pthread_mutex_init(&_mutex_pool, NULL);
    if(err != 0)
        goto cleanup1;
    
    err = atexit(&item_allocator_destroy);
    if(err != 0)
        goto cleanup2;
    
    return 0;

cleanup2:
    pthread_mutex_destroy(&_mutex_pool);
cleanup1:
    mempool_delete(_pool);
    _pool = NULL;
out:
    _pool_initialized = PTHREAD_ONCE_INIT;
    return  (err < 0) ? err : -err;
}

void item_allocator_destroy(void)
{
    if(_pool) {
        pthread_mutex_destroy(&_mutex_pool);
        mempool_delete(_pool);
        _pool_initialized = PTHREAD_ONCE_INIT;
        _pool = NULL;
    }
}

struct item *item_allocator_alloc(void)
{
    struct item *item;
    int err;
    
    err = pthread_once(&_pool_initialized, &_item_allocator_init);
    if(err != 0)
        return NULL;

    if(unlikely(!_pool))
        return NULL;
    
    pthread_mutex_lock(&_mutex_pool);
    item = mempool_alloc_chunk(_pool);
    pthread_mutex_unlock(&_mutex_pool);
    
    return item;
}

inline void item_allocator_free(struct item *__restrict item)
{
    pthread_mutex_lock(&_mutex_pool);
    mempool_free_chunk(_pool, item);
    pthread_mutex_unlock(&_mutex_pool);
}
