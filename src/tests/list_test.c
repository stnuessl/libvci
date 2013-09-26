#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <list.h>
#include <item.h>

int my_key_compare(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

void item_print(struct item *item)
{
    int *data, *key;
    
    data = item_data(item);
    key  = item_key(item);
    
    fprintf(stdout, "Data %d <> Key %d\n", *data, *key);
}

void test_insert(void)
{
    struct list *list;
    struct item *item;
    int *data, *key;
    int i, size;
    
    list = list_new();
    assert(list);
    
    size = 20;
    
    for(i = 0; i < size; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = i;
        *key  = i;
        
        item = item_new(data, key);
        assert(item);
        
        if(i & 1)
            list_insert_item_front(list, item);
        else
            list_insert_item_back(list, item);
    }
    
    list_for_each(list, item)
        item_print(item);
      
    assert(list_size(list) == size);

    list_delete(list, &free, &free);
}

void test_take(void)
{
    struct list *list;
    struct item *item;
    int *data, *key;
    int i, size;
    
    list = list_new();
    assert(list);
    
    size = 20;
    
    for(i = 0; i < size; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = i;
        *key  = i;
        
        item = item_new(data, key);
        assert(item);
       
        list_insert_item_front(list, item);
    }
    
    while(!list_empty(list)) {
        
        if(list_size(list) & 1)
            item = list_take_item_front(list);
        else
            item = list_take_item_back(list);
        
        item_print(item);
        item_delete(item, &free, &free);
    }
    
    assert(list_size(list) == 0);
    
    list_delete(list, NULL, NULL);
}

void test_sort(void)
{
    struct list list;
    struct item *item;
    int *data, *key;
    int size;
    
    list_init(&list);
    srand(getpid());
    
    size = 10;
    
    while(size--) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = size;
        *key  = random() % 10000;
        
        item = item_new(data, key);
        assert(item);
        
        list_insert_item_front(&list, item);
    }
    
    list_sort(&list, &my_key_compare);
    
    list_for_each(&list, item)
        item_print(item);
        
    list_destroy(&list, &free, &free);
}

void test_insert_sorted(void)
{
    struct list list;
    struct item *item;
    int *data, *key;
    int size;
    
    list_init(&list);
    srand(getpid());
    
    size = 30;
    
    while(size--) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = size;
        *key  = random() % 10000;
        
        item = item_new(data, key);
        assert(item);
        
        sorted_list_insert_item(&list, item, &my_key_compare);
    }
   
    list_for_each(&list, item) {
        key  = item_key(item);
        
        fprintf(stdout, "%d\n", *key);
    }
    
    list_destroy(&list, &free, &free);
}

void test_sorted(void)
{
    struct list list;
    struct item *item;
    int *data, *key;
    int i, size;
    
    list_init(&list);

    size = 30;
    i = size;
    
    while(i--) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = 1;
        *key  = i;
        
        item = item_new(data, key);
        assert(item);
        
        sorted_list_insert_item(&list, item, &my_key_compare);
    }
    
    for(i = 0; i < size; ++i) {
        item = sorted_list_retrieve_item(&list, &i, &my_key_compare);
        if(!item)
            abort();
    }
    
    while(i--) {
        i += 1000;
        
        data = sorted_list_retrieve(&list, &i, &my_key_compare);
        if(data)
            abort();
        
        i -= 1000;
    }
    
    
    list_destroy(&list, &free, &free);
}

int main(int argc, char *argv[])
{
    test_insert();
    test_take();
    test_sort();
    test_insert_sorted();
    test_sorted();
    
    fprintf(stdout, "List test(s) finished.\n");
   
    return EXIT_SUCCESS;
}