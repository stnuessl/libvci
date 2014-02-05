#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <libvci/cache.h>

unsigned int int_hash(const void *key)
{
    return (long)key;
}

int int_compare(const void *a, const void *b)
{
    return (long) a - (long) b;
}



void test_random(void)
{
#define N_ELEMENTS 1000000
    struct cache *cache;
    int fd, i, data, err;
    unsigned char key;
    unsigned int hit, miss;
    
    hit = 0;
    miss = 0;
    
    cache = cache_new(UCHAR_MAX / 2, &int_compare, &int_hash);
    assert(cache);
    
    fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    
    for(i = 0; i < UCHAR_MAX; i += 2)
        cache_insert(cache, (void *)(long)i, (void *)1);
    
    for(i = 0; i < N_ELEMENTS; ++i) {
        err = read(fd, &key, sizeof(key));
        assert(err == sizeof(key));
        
        data = (int)(long) cache_retrieve(cache, (void *)(long)key);
        if(data)
            hit += 1;
        else
            miss += 1;
    }
    
    fprintf(stdout, "Cache performance %d %%\n", 100 * hit / (hit + miss));
    
    close(fd);
    cache_delete(cache);
#undef N_ELEMENTS
}

void test_correctness(void)
{
#define CACHE_SIZE 10
#define N_ELEMENTS 20
    struct cache *cache;
    int i, data;
    
    cache = cache_new(CACHE_SIZE, &int_compare, &int_hash);
    assert(cache);

    for(i = 0; i < CACHE_SIZE; i++)
        cache_insert(cache, (void *)(long) i, (void *) 1);
    
    for(i = CACHE_SIZE; i < N_ELEMENTS; ++i)
        cache_insert(cache, (void *)(long) i, (void *) 1);
    
    for(i = 0; i < N_ELEMENTS; ++i) {
        data = (long) cache_retrieve(cache, (void *)(long) i);
        
        if(i < CACHE_SIZE)
            assert(!data);
        else
            assert(data);
    }

    cache_delete(cache);
#undef N_ELEMENTS
#undef CACHE_SIZE
}

int main(int argc, char *argv[])
{
    test_random();
    test_correctness();
    return EXIT_SUCCESS;
}