#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <avltree.h>
#include <clock.h>
#include <macros.h>

struct node {
    int data;
    struct avlnode avlnode;
};

int int_compare(const void *a, const void *b)
{
    return (long) a - (long) b;
}

void node_delete_from_avlnode(struct avlnode *avlnode)
{
    free(container_of(avlnode, struct node, avlnode));
}

void avltree_test_performance(int num)
{
    struct avltree *tree;
    struct clock *c;
    struct avlnode *avlnode;
    struct node *node, *tmp;
    int i, err;
    
    tree = avltree_new(&int_compare);
    c    = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    
    assert(tree);
    assert(c);
    
    avltree_set_data_delete(tree, &node_delete_from_avlnode);
    
    node = calloc(num, sizeof(*node));
    assert(node);
    
    clock_start(c);
    
    for(i = 0; i < num; ++i) {
        node[i].data = i;
        err = avltree_insert(tree, &node[i].avlnode, (void *)(long) i);
        assert(err == 0);
    }
    
    fprintf(stdout, 
            "Elapsed time for %d insertions is %lu ms.\n", 
             num, clock_elapsed_ms(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i) {
        avlnode = avltree_retrieve(tree, (void *)(long) i);
        tmp = container_of(avlnode, struct node, avlnode);
        assert(tmp->data == i);
    }
    
    fprintf(stdout, 
            "Elapsed time for %d retrievals is %lu ms.\n", 
            num, clock_elapsed_ms(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i) {
        avlnode = avltree_take(tree, (void *)(long) i);
        tmp = container_of(avlnode, struct node, avlnode);
        assert(tmp->data == i);
    }
    
    fprintf(stdout, 
            "Elapsed time for %d removals is %lu ms.\n", 
            num, clock_elapsed_ms(c));
    
    free(node);
    clock_delete(c);
    avltree_delete(tree);
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <number of elements>\n"
                        "Example: %s 10000\n",
                        argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    
    avltree_test_performance(atoi(argv[1]));
    
    return EXIT_SUCCESS;
}