#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <cache.h>


#define N_ELEMENTS 1000
#define CACHE_SIZE 1000



int int_compare(const void *a, const void *b)
{
    return (int) a - (int) b;
}

int main(int argc, char *argv[])
{
    struct cache *cache;
    int i, j;
    
    cache = cache_new(CACHE_SIZE, &int_compare, NULL);
    assert(cache);
    
    for(i = 1; i < N_ELEMENTS; ++i)
        cache_insert(cache, (void *)(long)i, (void *)(long)i);
    
    
    for(i = 1; i < N_ELEMENTS; ++i) {
        j = (int)(long) cache_retrieve(cache, (void *)(long)i);
        
        if(j)
            fprintf(stdout, "Cache HIT\n");
        else
            fprintf(stdout, "Cache MISS\n");
    }
    
    fprintf(stdout, "Cache performance %d %%\n", cache_performance(cache));
    
    cache_delete(cache);
    
    return EXIT_SUCCESS;
}