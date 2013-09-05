#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <mempool.h>

#define LOOPS 1
#define POOL_CHUNKS 2000
#define USED_CHUNKS 2000
#define DATA_SIZE 64

struct a {
    char a[DATA_SIZE];
};

#define TYPE struct a

#define timespec_to_double(ts)                                                 \
    (double) ((ts).tv_sec + (ts).tv_nsec * 1e-9)
    
#define timespec_diff(end, start)                                              \
    timespec_to_double(end) - timespec_to_double(start)

void test_with_mempool(void)
{
    struct mempool *p;
    TYPE *chunks[USED_CHUNKS];
    int i, j, err;
    struct timespec start, end;
    
    err = clock_gettime(CLOCK_MONOTONIC, &start);
    assert(err == 0);
    
    p = mempool_new(POOL_CHUNKS, sizeof(TYPE));
    assert(p);

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
    
    err = clock_gettime(CLOCK_MONOTONIC, &end);
    assert(err == 0);
    
    printf("Test with Memory Pool: %lf\n", timespec_diff(end, start));
}

void test_without_mempool(void)
{
    TYPE *chunks[USED_CHUNKS];
    int i, j, err;
    struct timespec start, end;
    
    err = clock_gettime(CLOCK_MONOTONIC, &start);
    assert(err == 0);
    
    for(i = 0; i < LOOPS; ++i) {
        
        for(j = 0; j < USED_CHUNKS; ++j) {
            chunks[j] = malloc(sizeof(*chunks[j]));
            assert(chunks[j]);
            memset(chunks[j], 0xff, sizeof(TYPE));
        }

        
        for(j = 0; j < USED_CHUNKS; ++j)
            free(chunks[j]);
    }
    
    err = clock_gettime(CLOCK_MONOTONIC, &end);
    assert(err == 0);
    
    printf("Test without Memory Pool: %lf\n", timespec_diff(end, start));
}

int main(int argc, char *argv[])
{
    test_without_mempool();
    test_with_mempool();
    
    return EXIT_SUCCESS;
}