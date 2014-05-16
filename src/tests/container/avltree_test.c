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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <libvci/avltree.h>
#include <libvci/clock.h>
#include <libvci/macro.h>

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

void avltree_test_deletion(void)
{
#define NUM_NODES 1000
    struct avltree *tree;
    struct node *node;
    int fd, i, rand, err;
    
    fd = open("/dev/urandom", O_RDONLY);
    if(fd < 0) {
        fprintf(stderr, "open() failed - %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    tree = avltree_new(&int_compare);
    assert(tree);
    
    avltree_set_data_delete(tree, &node_delete_from_avlnode);
    
    for(i = 0; i < NUM_NODES; ++i) {
        node = malloc(sizeof(*node));
        assert(node);
        
        err = read(fd, &rand, sizeof(rand));
        assert(err == sizeof(rand));
        
        node->data = rand;
        
        err = avltree_insert(tree, &node->avlnode, (void *)(long) i);
        if(err < 0) {
            if(err == -EINVAL) {
                fprintf(stdout, "Duplicate key -> %d\n", rand);
                free(node);
            } else {
                fprintf(stderr, "Unkown error ( %d ) on insertion.\n", err);
            }
        } 
    }
    
    close(fd);
    avltree_delete(tree);
}

void avltree_test_iteration_performance(void)
{
    struct avltree *tree;
    struct avlnode *avlnode;
    struct clock *c;
    struct node *node;
    int i, num, err;
    
    num = 1000000;
    
    tree = avltree_new(&int_compare);
    c    = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(tree);
    assert(c);
    
    avltree_set_data_delete(tree, &node_delete_from_avlnode);
    
    for(i = 0; i < num; ++i) {
        node = malloc(sizeof(*node));
        assert(node);
        
        node->data = i + 1;
        
        err = avltree_insert(tree, &node->avlnode, (void *)(long)node->data);
        assert(err == 0);
    }
    
    i = 0;
    
    clock_start(c);
    
    avltree_for_each_postorder(tree, avlnode) {
        node = container_of(avlnode, struct node, avlnode);
        
        assert(node->data != 0);
        
        i += 1;
    }
    
    fprintf(stdout, 
            "Elapsed time for iteration of %d elements: %lu us.\n",
            i, clock_elapsed_us(c));
    
    assert(i == num);
    
    avltree_delete(tree);
    clock_delete(c);
}

void avltree_test_iteration_print(void)
{
    struct avltree *tree;
    struct avlnode *avlnode;
    struct node *node;
    int i, num, err;
    
    num = 10;
    
    tree = avltree_new(&int_compare);
    assert(tree);
    
    avltree_set_data_delete(tree, &node_delete_from_avlnode);
    
    for(i = 0; i < num; ++i) {
        node = malloc(sizeof(*node));
        assert(node);
        
        node->data = i + 1;
        
        err = avltree_insert(tree, &node->avlnode, (void *)(long)node->data);
        assert(err == 0);
        
        avltree_for_each_postorder(tree, avlnode) {
            node = container_of(avlnode, struct node, avlnode);
            
            fprintf(stdout, "Value: %2d   ", node->data);
        }
        
        fprintf(stdout, "\n");
    }
    
    avltree_delete(tree);
}
void avltree_test_iteration(void)
{
    avltree_test_iteration_print();
    avltree_test_iteration_performance();
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
            "Elapsed time for %d insertions is %lu us.\n", 
             num, clock_elapsed_us(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i) {
        avlnode = avltree_retrieve(tree, (void *)(long) i);
        tmp = container_of(avlnode, struct node, avlnode);
        assert(tmp->data == i);
    }
    
    fprintf(stdout, 
            "Elapsed time for %d retrievals is %lu us.\n", 
            num, clock_elapsed_us(c));
    
    clock_reset(c);
    
    for(i = 0; i < num; ++i) {
        avlnode = avltree_take(tree, (void *)(long) i);
        tmp = container_of(avlnode, struct node, avlnode);
        assert(tmp->data == i);
    }
    
    fprintf(stdout, 
            "Elapsed time for %d removals is %lu us.\n", 
            num, clock_elapsed_us(c));
    
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
    avltree_test_deletion();
    avltree_test_iteration();
    
    return EXIT_SUCCESS;
}