#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "mempool.h"

struct chunk {
    struct chunk *next;
};

static int _mempool_grow(struct mempool *__restrict pool, int size)
{
    void **mem;
    void *chunk;
    int num, i, err;
    
    mem = realloc(pool->_mem, size * sizeof(*mem));
    if(!mem)
        return -errno;
    
    for(i = pool->_size; i < size; ++i) {
        mem[i]     = calloc(pool->_num_chunks, pool->_chunk_size);
        if(!mem[i]) {
            err = -errno;
            goto cleanup1;
        }
    }
    
    for(i = pool->_size; i < size; i += 2) {
        chunk = mem[i];
        num   = pool->_num_chunks;
        
        while(num--) {
            /* add 'chunk' to list */
            ((struct chunk *)chunk)->next = pool->_free_chunks;
            pool->_free_chunks = chunk;

            chunk += pool->_chunk_size;
        }
    }
    
    pool->_mem  = mem;
    pool->_size = size;
    
    return 0;
    
cleanup1:
    while(i-- > pool->_size)
        free(mem[i]);
    
    return err;
}

struct mempool *mempool_new(int num_chunks, size_t chunk_size)
{
    struct mempool *pool;
    int err;
    
    pool = malloc(sizeof(*pool));
    if(!pool)
        return NULL;
    
    err = mempool_init(pool, num_chunks, chunk_size);
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
                 int num_chunks, 
                 size_t chunk_size)
{
    memset(pool, 0, sizeof(*pool));
    
    pool->_num_chunks = num_chunks;
    pool->_chunk_size = max(sizeof(struct chunk), chunk_size);
    
    return _mempool_grow(pool, 1);
}

void mempool_destroy(struct mempool *__restrict pool)
{
    while(pool->_size--)
        free(pool->_mem[pool->_size]);

    free(pool->_mem);
}

void *mempool_alloc_chunk(struct mempool *__restrict pool)
{
    struct chunk *chunk;
    
    if(mempool_empty(pool)) {
        if(_mempool_grow(pool, pool->_size << 1) < 0)
            return NULL;
    }
    
    /* remove 'chunk' from the list */
    chunk = pool->_free_chunks;
    pool->_free_chunks = ((struct chunk *)pool->_free_chunks)->next;
    
    return chunk;
}

void mempool_free_chunk(struct mempool *__restrict pool, void *chunk)
{
    /* add 'chunk' to the list */
    ((struct chunk *)chunk)->next = pool->_free_chunks;
    pool->_free_chunks = chunk;
}

inline bool mempool_empty(struct mempool *__restrict pool)
{
    return pool->_free_chunks == NULL;
}