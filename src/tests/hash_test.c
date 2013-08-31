#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <hash.h>
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
    "hash",
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
#define HASH_SIZE 10
#define NELEMENTS 20
    struct hash *hash;
    struct item *item;
    long *key, tmp;
    int *data, i, err;
    char *s1, *s2;
    
    fprintf(stdout, "Hash test started.\n");
    
    /* create and initizalize hash */
    hash = hash_new(HASH_SIZE, sizeof(*key));
    
    assert(hash);
    
    hash_set_data_delete(hash, &free);
    hash_set_key_delete(hash, &free);
    hash_set_key_compare(hash, &my_key_compare);
    
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
        
        hash_insert_item(hash, item);
    }
    
    hash_for_each(hash, item) {
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "Data %d, key %li\n", *data, *key);
    }
    
    for(i = 0; i < (NELEMENTS >> 1); ++i) {
        tmp = i;
        item = hash_take_item(hash, &tmp);
        assert(item);
        
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "Took item with key %li: data is %d\n", *key, *data);
        item_delete(item, &free, &free);
    }
    
    for(i = NELEMENTS >> 1; i < NELEMENTS; ++i) {
        tmp = i;
        item = hash_retrieve_item(hash, &tmp);
        assert(item);
    }
    
    hash_for_each(hash, item) {
        data = item_data(item);
        key  = item_key(item);
        
        fprintf(stdout, "data %d, key %li\n", *data, *key);
    }
    
    hash_delete(hash);
    
    fprintf(stdout, "Starting hash string test.\n");
    
    hash = hash_new(2, 0);
    assert(hash);
    
    hash_set_key_length(hash, &my_key_length);
    hash_set_key_compare(hash, &my_string_compare);
    
    for(i = 0; strings[i]; ++i) {
        err = hash_insert(hash, strings[i], keys[i]);
        assert(!err);
    }
    
    s1 = hash_retrieve(hash, "3");
    assert(s1);
    
    fprintf(stdout, "Data of key %s is %s\n", keys[3], s1);
    
    hash_for_each(hash, item) {
        s1 = item_key(item);
        s2 = item_data(item);
        
        fprintf(stdout, "%s - %s\n", s1, s2);
    }
    hash_delete(hash);
    
    fprintf(stdout, "Hash test finished.\n");
    
    return EXIT_SUCCESS;
    
#undef HASH_SIZE
#undef NELEMENTS
}