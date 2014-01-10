#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "mempool.h"


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
    unsigned long *last;
    
    pool->mem        = mem;
    pool->init       = mem;
    pool->next       = mem;
    pool->size       = size;
    pool->chunk_size = max(sizeof(unsigned long), chunk_size);
    pool->chunks     = pool->size / pool->chunk_size; 
    
    last = pool->mem + (pool->chunk_size * (pool->chunks - 1));
    
    *last = (unsigned long) NULL;
    
    return 0;
}

void mempool_destroy(struct mempool *__restrict pool)
{
    /* make the pool unusable */
    memset(pool, 0, sizeof(*pool));
}

void* mempool_alloc_chunk(struct mempool *__restrict pool)
{
    unsigned long *chunk;
    
    if(pool->init < pool->mem + pool->size) {
        chunk = pool->init;
        *chunk = (unsigned long) pool->init + pool->chunk_size;
        pool->init += pool->chunk_size;
    }
    
    if(unlikely(mempool_empty(pool)))
        return malloc(pool->chunk_size);
    
    pool->chunks -= 1;
    
    chunk = pool->next;
    pool->next = (void *)*chunk;
    
    return chunk;
}

void mempool_free_chunk(struct mempool *__restrict pool, void *chunk)
{
    unsigned long *next;
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
    
    pool->chunks += 1;
    
    next = chunk;
    
    *next = (unsigned long) pool->next;
    pool->next = next;
}

inline bool mempool_empty(const struct mempool *__restrict pool)
{
    return pool->chunks == 0;
}