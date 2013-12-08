#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "mempool.h"


struct chunk {
    struct chunk *next;
};


struct mempool *mempool_new(void *mem, size_t size, size_t chunk_size)
{
    struct mempool *pool;
    int err;
    
    pool = malloc(sizeof(*pool));
    if(!pool)
        return NULL;
    
    err = mempool_init(pool, mem, size, chunk_size);
    if(err < 0) {
        free(pool);
        return NULL;
    }
    
    return pool;
}

void mempool_delete(struct mempool *__restrict pool)
{
    mempool_destroy(pool);
    free(pool);
}

int mempool_init(struct mempool *__restrict pool, 
                 void *mem, 
                 size_t size, 
                 size_t chunk_size)
{
    pool->mem         = mem;
    pool->size        = size;
    pool->chunk_size  = max(sizeof(struct chunk), chunk_size);
    pool->mem_used    = 0;
    pool->list_chunks = NULL;
    
    return 0;
}

void mempool_destroy(struct mempool *__restrict pool)
{
    /* make the pool unusable */
    memset(pool, 0, sizeof(*pool));
}

void* mempool_alloc_chunk(struct mempool *__restrict pool)
{
    void *chunk;
    
    if(pool->list_chunks != NULL) {
        /* remove chunk from list */
        chunk = pool->list_chunks;
        pool->list_chunks = ((struct chunk *)pool->list_chunks)->next;
        
        return chunk;
    }
    
    if(unlikely(pool->mem_used >= pool->size))
        return malloc(pool->chunk_size);
    
    chunk = pool->mem + pool->mem_used;    
    pool->mem_used += pool->chunk_size;
    
    return chunk;
}

void mempool_free_chunk(struct mempool *__restrict pool, void *chunk)
{
    /* 
     * We can risk to further trash the performance for an empty mempool
     * (therefore getting a slighty better performance for non-empty pools)
     * because the programmer should be able to detect this 
     * (e.g. 'mempool_empty()') and increase to memory size of the pool 
     * accordingly.
     */
    if(unlikely(chunk < pool->mem || chunk >= pool->mem + pool->size)) {
        free(chunk);
        return;
    }
    
    /* insert again into list */
    ((struct chunk *)chunk)->next = pool->list_chunks;
    pool->list_chunks = chunk;
}

inline bool mempool_empty(const struct mempool *__restrict pool)
{
    return pool->mem_used >= pool->size && pool->list_chunks == NULL;
}