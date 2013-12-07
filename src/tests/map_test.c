#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>


#include <map.h>
#include <clock.h>

int int_compare(const void *a, const void *b)
{
    return (long)a - (long)b;
}

size_t string_length(const void *a)
{
    return strlen(a);
}

int string_compare(const void *a, const void *b)
{
    return strcmp(a, b);
}

unsigned int hash(const void *key)
{
    unsigned int hval;
    int val;
    
    val = (int)(long)key;
    hval = 1;  

    hval += val;
    hval += (hval << 10);
    hval ^= (hval >> 6);
    hval &= 0x0fffffff;
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    // fprintf(stdout, "Hash value of %d is %u.\n", val, hval);
    
    return hval;
}

char *strings[] = {
    "I",
    "hope",
    "this",
    "map",
    "thing",
    "works",
    NULL
};

char *keys[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    NULL
};

void inspect_map(const struct map *__restrict map)
{
    unsigned int i;
    
    for(i = 0; i < map->capacity; ++i) {
        if(map->table[i].state == DATA_AVAILABLE)
            fprintf(stdout, "At index %d: Key %d -> Value %d -> Hash %u\n",
                    i,
                    (int)(long) map->table[i].key,
                    (int)(long) map->table[i].data,
                    map->table[i].hash
                   );
    }
    
    fprintf(stdout, "Map entries: %u.\n"
                    "Map capacity: %u.\n"
                    "Map occupation is %d %%.\n",
            map->entries, 
            map->capacity,
            100 * map->entries / map->capacity);
}

void map_test_insert_remove(void)
{
    struct map *map;
    unsigned int num_elements;
    int i, err;
    
    num_elements = 32;
    
    map = map_new(0, &int_compare, &hash);
    assert(map);
    
    for(i = 0; i < num_elements; ++i) {
        err = map_insert(map, (void *)(long)i, (void *)(long)i);
        assert(err == 0);
    }
    
    for(i = 0; i < num_elements - (num_elements >> 1); ++i)
        assert((int)(long)map_take(map, (void *)(long)i) == i);
    
    inspect_map(map);
    
    map_delete(map);
}

void map_test_performance(unsigned int num)
{
    struct map *map;
    struct clock *c;
    int i, err;
    
    map = map_new(0, &int_compare, &hash);
    c   = clock_new(CLOCK_MONOTONIC);
    assert(map);
    assert(c);
    
    clock_start(c);
    
    for(i = 0; i < num; ++i) {
        err = map_insert(map, (void *)(long) i, (void *)(long) i);
        assert(err == 0);
    }
    
    fprintf(stdout, "Elapsed time for %u insertions: %lu ms\n",
            num, 
            clock_elapsed_ms(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i)
        assert((int)(long) map_retrieve(map, (void *)(long) i) == i);
    
    fprintf(stdout, "Elapsed time for %u lookups: %lu ms\n",
            num,
            clock_elapsed_ms(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i)
        assert((int)(long) map_take(map, (void *)(long) i) == i);
    
    fprintf(stdout, "Elapsed time for %u removals: %lu ms\n",
            num,
            clock_elapsed_ms(c));
    
    clock_delete(c);
    map_delete(map);
}

void stress_test(void)
{
    struct map *map;
    int err, i, num_elements, loops;
    
    map = map_new(0, &int_compare, &hash);
    assert(map);
    
    loops = 1000;
    num_elements = 100000;
    
    while(loops--) {
        for(i = 0; i < num_elements / 2; ++i) {            
            err = map_insert(map, (void *)(long)i, (void *)(long) i);
            assert(err == 0);
        }
        
        for(i = 0; i < num_elements / 4; i += 4)
            assert((int)(long)map_take(map, (void *)(long)i) == i);
        
        for(i = num_elements / 2; i < num_elements; ++i) {
            err = map_insert(map, (void *)(long)i, (void *)(long) i);
            assert(err == 0);
        }
        
        for(i = num_elements / 2; i < num_elements / 4; i += 4)
            assert((int)(long)map_take(map, (void *)(long)i) == i);
        
        map_clear(map);
    }
    
    map_delete(map);
}

int main(int argc, char *argv[])
{
    unsigned int num;
    
    if(argc != 2) {
        fprintf(stderr, "fatal: Missing argument(s).\n"   
                        "Usage: %s <number of elements>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    
    num = (unsigned int) atoi(argv[1]);
    
    map_test_insert_remove();
    map_test_performance(num);
    stress_test();
    
    return EXIT_SUCCESS;
}