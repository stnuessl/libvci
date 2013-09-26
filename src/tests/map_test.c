#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <map.h>
#include <item.h>

int my_key_compare(const void *a, const void *b)
{
    return *(long *)a - *(long *)b;
}

size_t my_key_length(const void *a)
{
    return strlen(a);
}

int my_string_compare(const void *a, const void *b)
{
    return strcmp(a, b);
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

int main(int argc, char *argv[])
{
#define MAP_SIZE 10
#define NELEMENTS 20
    struct map *map;
    struct item *item;
    long *key, tmp;
    int *data, i, err;
    char *s1, *s2;
    
    fprintf(stdout, "Map test started.\n");
    
    /* create and initizalize map */
    map = map_new(MAP_SIZE, sizeof(*key));
    
    assert(map);
    
    map_set_data_delete(map, &free);
    map_set_key_delete(map, &free);
    map_set_key_compare(map, &my_key_compare);
    
    /* add a bunch of elements */
    for(i = 0; i < NELEMENTS; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = i*i;
        *key  = i;
        
        item = item_new(data, key);
        
        assert(item);
        
        map_insert_item(map, item);
    }
    
    map_for_each(map, item) {
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "Data %d, key %li\n", *data, *key);
    }
    
    for(i = 0; i < (NELEMENTS >> 1); ++i) {
        tmp = i;
        item = map_take_item(map, &tmp);
        assert(item);
        
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "Took item with key %li: data is %d\n", *key, *data);
        item_delete(item, &free, &free);
    }
    
    for(i = NELEMENTS >> 1; i < NELEMENTS; ++i) {
        tmp = i;
        item = map_retrieve_item(map, &tmp);
        assert(item);
    }
    
    map_for_each(map, item) {
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "data %d, key %li\n", *data, *key);
    }
    
    map_delete(map);
    
    fprintf(stdout, "Starting map string test.\n");
    
    map = map_new(2, 0);
    assert(map);
    
    map_set_key_length(map, &my_key_length);
    map_set_key_compare(map, &my_string_compare);
    
    for(i = 0; strings[i]; ++i) {
        err = map_insert(map, strings[i], keys[i]);
        assert(!err);
    }
    
    s1 = map_retrieve(map, "3");
    assert(s1);
    
    fprintf(stdout, "Data of key %s is %s\n", keys[3], s1);
    
    map_for_each(map, item) {
        s1 = item_key(item);
        s2 = item_data(item);
        
        fprintf(stdout, "%s - %s\n", s1, s2);
    }
    map_delete(map);
    
    fprintf(stdout, "Map test finished.\n");
    
    return EXIT_SUCCESS;
    
#undef MAP_SIZE
#undef NELEMENTS
}