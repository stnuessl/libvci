#include <errno.h>

#include "mempool.h"
#include "item.h"
#include "macros.h"
#include "item_allocator.h"

#define ITEM_ALLOCATOR_SIZE 1000

static struct mempool *_pool;

static struct mempool *_item_allocator(void)
{
    if(unlikely(!_pool)) {
        if(item_allocator_init(ITEM_ALLOCATOR_SIZE) < 0)
            return NULL;
    }
    
    return _pool;
}

int item_allocator_init(int size)
{
    int err;
    
    _pool = mempool_new(size, sizeof(struct item));
    if(!_pool)
        return -errno;
    
    err = atexit(&item_allocator_destroy);
    if(err != 0) {
        mempool_delete(_pool);
        _pool = NULL;
        return (err < 0) ? err : -err;
    }
    
    return 0;
}

void item_allocator_destroy(void)
{
    if(_pool) {
        mempool_delete(_pool);
        _pool = NULL;
    }
}

struct item *item_allocator_alloc(void)
{
    struct mempool *pool;
    
    pool = _item_allocator();
    if(unlikely(!pool))
        return NULL;
    
    return mempool_alloc_chunk(pool);
}

inline void item_allocator_free(struct item *__restrict item)
{
    mempool_free_chunk(_pool, item);
}