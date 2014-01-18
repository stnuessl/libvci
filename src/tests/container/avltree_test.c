#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <avltree.h>
#include <macros.h>

struct node {
    int data;
    struct avlnode avlnode;
};

int int_compare(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

int main(int argc, char *argv[])
{
#define NUM_ELEMENTS 100
    struct avltree *tree;
    struct node *node;
    struct avlnode *avlnode;
    int i, err;
    
    tree = avltree_new(&int_compare);
    assert(tree);
    
    for(i = 0; i < NUM_ELEMENTS; ++i) {
        node = malloc(sizeof(*node));
        assert(node);
        
        node->data = i;
        
        err = avltree_insert(tree, &node->avlnode, &node->data);
        assert(err == 0);
    }
    
    for(i = 0; i < NUM_ELEMENTS; ++i) {
        avlnode = avltree_retrieve(tree, &i);
        assert(avlnode);
    }
    
    for(i = 0; i < NUM_ELEMENTS; ++i) {
        avlnode = avltree_take(tree, &i);
        node = container_of(avlnode, struct node, avlnode);
        free(node);
    }
    
    avltree_delete(tree);
    
    return EXIT_SUCCESS;
}