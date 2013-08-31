#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <tree.h>
#include <bintree.h>
#include <item.h>

int my_key_compare(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}


int main(int argc, char *argv[])
{
#define NELEMENTS 100
    struct tree tree;
    struct item *item;
    int *data, *key, i;
    
    fprintf(stdout, "Bintree test started.\n");
    
    tree_init(&tree);
    
    tree_set_key_compare(&tree, &my_key_compare);
    tree_set_data_delete(&tree, &free);
    tree_set_key_delete(&tree, &free);
    
    srandom(getpid());
    
    for(i = 0; i < NELEMENTS; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));
        
        assert(data);
        assert(key);
        
        *data = random() % 300;
        *key  = i % 200;
        
        item = item_new(data, key);
        
        assert(bintree_insert_item(&tree, item));
    }
    
    assert(NELEMENTS == tree_size(&tree));
    
    tree_destroy(&tree);
    
    fprintf(stdout, "Bintree test finished.\n");
    
    return EXIT_SUCCESS;
}