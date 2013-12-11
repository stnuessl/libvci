#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include <stdlib.h>
#include <stdbool.h>

struct mempool {
    void *mem;
    size_t size;
    size_t chunk_size;
    
    size_t mem_used;
    void *list_chunks;
};

struct mempool *mempool_new(void *mem, size_t size, size_t chunk_size);

void mempool_delete(struct mempool *__restrict pool);

int mempool_init(struct mempool *__restrict pool, 
                 void *mem, 
                 size_t size, 
                 size_t chunk_size);

void mempool_destroy(struct mempool *__restrict pool);

void* mempool_alloc_chunk(struct mempool *__restrict pool);

void mempool_free_chunk(struct mempool *__restrict pool, void *chunk);

inline bool mempool_empty(const struct mempool *__restrict pool);


struct mempool_old {
    void *_free_chunks;
    void **_mem;
    size_t _chunk_size;
    int _num_chunks;
    
    int _size;
};

#endif /* _MEMPOOL_H_ */