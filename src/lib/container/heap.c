#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "item.h"
#include "list.h"
#include "tree.h"
#include "heap.h"

static unsigned int _heap_path(int position)
{
    int i, path;
    
    path = 0;
    
    /* reverse bits, align them right */
    for(i = position; i; i >>= 1) {
        path <<= 1;
        path |= (i & 0x01);
    }
    
    /* we don't need the last bit */
    return path >>=1;
}

static struct item **_heap_ref_item(struct tree *__restrict tree, int position)
{
    struct item **item;
    unsigned int path;
    int i;
    
    item = &tree->_root;
    
    path = _heap_path(position);
    
    for(i = position >> 1; i; i >>= 1) {
        if(path & 0x01)
            item = item_ref_right(*item);
        else
            item = item_ref_left(*item);
        
        path >>= 1;
    }
    
    return item;
}

static void _heap_insert_item(struct tree *__restrict tree, 
                              struct item *item,
                              int position)
{
    struct item **ref;
    int path, i;
    
    ref = &tree->_root;
    
    path = _heap_path(position);
    
    for(i = position >> 1; i; i >>= 1) {
        if(tree->_key_compare(item_key(item), item_key(*ref)) < 0)
            item_swap(item, *ref);
        
        if(path & 0x01)
            ref = item_ref_right(*ref);
        else
            ref = item_ref_left(*ref);
        
        path >>= 1;
    }
    
    *ref = item;
}

static void _tree_heapify_item(struct tree *__restrict tree, 
                               struct item **root)
{
    struct item **left, **right, **min;
    
    if(!*root)
        return;
    
    min = root;
    
    left  = item_ref_left(*root);
    right = item_ref_right(*root);
    
    if(*left) {
        if(tree->_key_compare(item_key(*left), item_key(*min)) < 0)
            min = left;
    }
    
    if(*right) {
        if(tree->_key_compare(item_key(*right), item_key(*min)) < 0)
            min = right;
    }
    
    if(*min != *root) {
        item_swap(*min, *root);
        _tree_heapify_item(tree, min);
    }
}

int heap_insert(struct tree *__restrict tree, 
                void *__restrict data, 
                void *__restrict key)
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    heap_insert_item(tree, item);
    
    return 0;
}

void heap_insert_item(struct tree *__restrict tree, 
                      struct item *__restrict item)
{
    _heap_insert_item(tree, item, ++tree->_size);
}

void *heap_take_root(struct tree *__restrict tree)
{
    struct item *item;
    void *data;
    
    item = heap_take_root_item(tree);
    
    data = item_data(item);
    
    item_delete(item, NULL, tree->_key_delete);
    
    return data;
}

struct item *heap_take_root_item(struct tree *__restrict tree)
{
    struct item **item, *tmp;
    
    item = _heap_ref_item(tree, tree->_size--);
    
    if(*item != tree->_root)
        item_swap(*item, tree->_root);
    
    tmp = *item;
    
    *item = NULL;
    
    _tree_heapify_item(tree, &tree->_root);
    
    return tmp;
}

void *heap_root(struct tree *__restrict tree)
{
    return item_data(heap_root_item(tree));
}

struct item *heap_root_item(struct tree *__restrict tree)
{
    return tree->_root;
}

struct list heap_sort(struct tree *__restrict tree)
{
    struct list list;
    
    list_init(&list);
    
    while(!tree_empty(tree))
        list_insert_item_back(&list, heap_take_root_item(tree));
    
    return list;
}