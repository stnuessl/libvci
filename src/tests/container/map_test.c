/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <libvci/map.h>
#include <libvci/clock.h>
#include <libvci/macro.h>

int int_compare(const void *a, const void *b)
{
    return (long)a - (long)b;
}

int string_compare(const void *a, const void *b)
{
    return strcmp(a, b);
}

void inspect_map(const struct map *__restrict map)
{
    struct entry *e;
    unsigned int i;
    
    for(i = 0; i < map->capacity; ++i) {
        if(map->table[i].state == MAP_DATA_STATE_AVAILABLE)
            fprintf(stdout, "At index %2d: Key %d -> Value %d -> Hash %u\n",
                    i, 
                    (int)(long) map->table[i].key,
                    (int)(long) map->table[i].data,
                    map->table[i].hash);
    }
    
    fprintf(stdout, "Inserted values: ");
    
    map_for_each(map, e)
        fprintf(stdout, "%u ", (unsigned int) (long) entry_data(e));
    
    fprintf(stdout, "\n");

    fprintf(stdout, "Map entries: %u / capacity: %u / occupation: %d %%.\n",
            map->size, map->capacity, 100 * map->size / map->capacity);
}

void map_test_insert_remove(void)
{
    struct map *map;
    unsigned int num_elements;
    int i, err;
    
    num_elements = 32;
    
    map = map_new(0, &int_compare, &hash_s64);
    assert(map);
    
    for(i = 0; i < num_elements; ++i) {
        err = map_insert(map, (void *)(long)i, (void *)(long) i);
        assert(err == 0);
    }
    
    for(i = 0; i < num_elements - (num_elements >> 1); ++i)
        assert((int)(long)map_take(map, (void *)(long) i) == i);
    
    inspect_map(map);
    
    map_delete(map);
}

void map_test_performance(unsigned int num)
{
    struct map *map;
    struct clock *c;
    int i, err;
    
    map = map_new(0, &int_compare, &hash_u32);
    c   = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(map);
    assert(c);
    
    clock_start(c);
    
    for(i = 0; i < num; ++i) {
        err = map_insert(map, (void *)(long) i, (void *)(long) i);
        assert(err == 0);
    }
    
    fprintf(stdout, "Elapsed time for %u insertions: %lu us\n",
            num, 
            clock_elapsed_us(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i)
        assert((int)(long) map_retrieve(map, (void *)(long) i) == i);
    
    fprintf(stdout, "Elapsed time for %u lookups: %lu us\n",
            num,
            clock_elapsed_us(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i)
        assert((int)(long) map_take(map, (void *)(long) i) == i);
    
    fprintf(stdout, "Elapsed time for %u removals: %lu us\n",
            num,
            clock_elapsed_us(c));
    
    clock_delete(c);
    map_delete(map);
    
    map = map_new(0, &int_compare, &hash_u32);
    c   = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(map);
    assert(c);
    
    clock_start(c);
    
    err = map_rehash(map, num);
    assert(err == 0);
    
    for(i = 0; i < num; ++i) {
        err = map_insert(map, (void *)(long) i, (void *)(long) i);
        assert(err == 0);
    }
    
    fprintf(stdout, 
            "Elapsed time for %u insertions with "
            "preallocated map memory: %lu us\n",
            num, 
            clock_elapsed_us(c));
    
    clock_delete(c);
    map_delete(map);
}

void map_stress_test(void)
{
    struct map *map;
    int err, i, num_elements;
    
    map = map_new(0, &int_compare, &hash_u64);
    assert(map);
    
    num_elements = 1000000;
    
    for(i = 0; i < num_elements / 2; ++i) {            
        err = map_insert(map, (void *)(long)i, (void *)(long) i);
        assert(err == 0);
    }
    
    for(i = 0; i < num_elements / 4; i += 3)
        assert((int)(long)map_take(map, (void *)(long)i) == i);
    
    for(i = 0; i < num_elements / 4; i += 3) {
        if((i % 3) == 0)
            assert((int)(long)map_retrieve(map, (void *)(long) i) == 0);
        else
            assert((int)(long)map_retrieve(map, (void *)(long) i) == i);
    }
    
    for(i = num_elements / 2; i < num_elements; ++i) {
        err = map_insert(map, (void *)(long)i, (void *)(long) i);
        assert(err == 0);
    }
    
    for(i = num_elements / 2; i < num_elements / 4; i += 3)
        assert((int)(long)map_take(map, (void *)(long)i) == i);
    
    for(i = num_elements / 2; i < num_elements / 4; i += 3) {
        if((i % 3) == 0)
            assert((int)(long)map_retrieve(map, (void *)(long) i) == 0);
        else
            assert((int)(long)map_retrieve(map, (void *)(long) i) == i);
    }
    
    map_clear(map);
    
    map_delete(map);
}

void map_string_test(void)
{
    char *strings[] = {
        "I",            "1",
        "hope",         "2",
        "this",         "3",
        "map",          "4",
        "thing",        "5",
        "works",        "6"
    };
    
    struct map *map;
    int err, i;
    
    map = map_new(0, &string_compare, &hash_string);
    assert(map);
    
    for(i = 0; i < ARRAY_SIZE(strings); i += 2) {
        err = map_insert(map, strings[i + 1], strings[i]);
        assert(err == 0);
    }
    
    for(i = 0; i < ARRAY_SIZE(strings); i += 2)
        fprintf(stdout, "%s ", (char *)map_retrieve(map, strings[i + 1]));
    
    fprintf(stdout, "\n");
     
    map_delete(map);
}

int main(int argc, char *argv[])
{

    map_test_insert_remove();
    
    if(argc == 2)
        map_test_performance((unsigned int) atoi(argv[1]));
    
    map_stress_test();
    map_string_test();
    
    return EXIT_SUCCESS;
}