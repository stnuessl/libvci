#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <mempool.h>
#include <clock.h>
#include <macro.h>

#define LOOPS 1
#define POOL_CHUNKS 2000
#define USED_CHUNKS 2000
#define DATA_SIZE 64

struct a {
    char a[DATA_SIZE];
};

#define TYPE struct a

char mem[POOL_CHUNKS * sizeof(TYPE)];

void test_with_mempool(void)
{
    struct mempool *p;
    struct clock *c;
    TYPE *chunks[USED_CHUNKS];
    int i, j;
    
    p = mempool_new(mem, ARRAY_SIZE(mem), sizeof(TYPE));
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(p);
    assert(c);
    
    clock_start(c);

    for(i = 0; i < LOOPS; ++i) {

        for(j = 0; j < USED_CHUNKS; ++j) {
            chunks[j] = mempool_alloc_chunk(p);
            assert(chunks[j]);
            memset(chunks[j], 0xff, sizeof(TYPE));
        }

        for(j = 0; j < USED_CHUNKS; ++j)
            mempool_free_chunk(p, chunks[j]);
    }

    mempool_delete(p);
    
    printf("Test with Memory Pool: %lu us\n", clock_elapsed_us(c));
    
    clock_delete(c);
}

void test_without_mempool(void)
{
    TYPE *chunks[USED_CHUNKS];
    struct clock *c;
    int i, j;
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(c);
    
    clock_start(c);
    
    for(i = 0; i < LOOPS; ++i) {
        
        for(j = 0; j < USED_CHUNKS; ++j) {
            chunks[j] = malloc(sizeof(*chunks[j]));
            assert(chunks[j]);
            memset(chunks[j], 0xff, sizeof(TYPE));
        }

        
        for(j = 0; j < USED_CHUNKS; ++j)
            free(chunks[j]);
    }
    
    printf("Test without Memory Pool: %lu us\n", clock_elapsed_us(c));
    
    clock_delete(c);
}

int main(int argc, char *argv[])
{

    if(argc == 2) {
        if(strcmp(argv[1], "--no-mempool") == 0)
            test_without_mempool();
        else if(strcmp(argv[1], "--mempool") == 0)
            test_with_mempool();
    } else {
        test_without_mempool();
        test_with_mempool();
    }

    return EXIT_SUCCESS;
}