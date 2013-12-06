#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <item.h>
#include <tree.h>
#include <heap.h>

int int_compare(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

void my_print(struct item *item, void *arg)
{
    int *data, *key;
    
    data = item_data(item);
    key  = item_key(item);
    
    fprintf(stdout, "Data %d, key %d\n", *data, *key);
}

int main(int argc, char *argv[])
{
#define NELEMENTS 30
    struct tree *heap;
    struct list list;
    struct item *item;
    int *data, *key;
    int i;
    
    heap = tree_new();
    assert(heap);
    
    tree_set_data_delete(heap, &free);
    tree_set_key_delete(heap, &free);
    tree_set_key_compare(heap, &int_compare);
    tree_set_callback(heap, &my_print);
    
    for(i = 0; i < NELEMENTS; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = i * i;
        *key  = i;
        
        item = item_new(data, key);
        
        assert(item);
        
        heap_insert_item(heap, item);
    }
    
    tree_pre_order(heap, NULL);
    
    list = heap_sort(heap);
    
    tree_delete(heap);
    
    fprintf(stdout, "List\n");
    
    list_for_each(&list, item)
        my_print(item, NULL);

    list_destroy(&list, &free, &free);
    
    return EXIT_SUCCESS;
}