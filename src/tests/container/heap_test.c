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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

#include <libvci/heap.h>
#include <libvci/clock.h>
#include <libvci/macro.h>
#include <libvci/link.h>
#include <libvci/list.h>
#include <libvci/avltree.h>

#define MAX_PRINT 30


struct list_node {
    struct link link;
    int val;
};

struct tree_node {
    struct avlnode avlnode;
    int val;
};

static int _int_compare(const void *a, const void *b)
{
    return (long) a - (long) b;
}

void heap_check(const struct heap *__restrict heap)
{
    unsigned int i, left, right;
    
    for(i = 0; i < (heap->size >> 1); ++i) {
        left  = 2 * i + 1;
        right = 2 * i + 2;
        
        if(left < heap->size)
            assert((long) heap->data[i] > (long) heap->data[left]);
        
        if(right < heap->size)
            assert((long) heap->data[i] > (long) heap->data[right]);
    }
}

void test_avltree_performance(int *data, unsigned int size)
{
    struct avltree *tree;
    struct avlnode *avlnode;
    struct tree_node *nodes, *tmp, *last;
    struct clock *c;
    unsigned int i;
    int err;
    
    nodes = malloc(size * sizeof(*nodes));
    assert(nodes);
    
    for(i = 0; i < size; ++i)
        nodes[i].val = data[i];
    
    tree = avltree_new(&_int_compare);
    c    = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(tree);
    assert(c);
    
    clock_start(c);
    
    for(i = 0 ;  i < size; ++i) {
        tmp = nodes + i;
        err = avltree_insert(tree, &tmp->avlnode, (void *)(long) tmp->val);
        assert(err == 0 || err == -EINVAL);
    }
    
    fprintf(stdout, 
            "Elapsed time for %u avltree insertions: %lu us.\n",
             size, clock_elapsed_us(c));
    
    clock_reset(c);
    
    if(avltree_size(tree)) {
        avlnode = avltree_take_max(tree);
        last = container_of(avlnode, struct tree_node, avlnode);
        
        while(!avltree_empty(tree)) {
            avlnode = avltree_take_max(tree);
            tmp = container_of(avlnode, struct tree_node, avlnode);
            assert(last->val >= tmp->val);
            last = tmp;
        }
    }
    
    fprintf(stdout, 
            "Elapsed time for %u avltree max removals: %lu us.\n",
            size, clock_elapsed_us(c));
    
    avltree_delete(tree);
    clock_delete(c);
    free(nodes);
}

void test_sorted_list_performance(int *data, unsigned int size)
{
    struct link list, *link;
    struct list_node *nodes, *tmp, *last;
    struct clock *c;
    unsigned int i;
    
    nodes = malloc(size * sizeof(*nodes));
    assert(nodes);
    
    for(i = 0; i < size; ++i)
        nodes[i].val = data[i];
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(c);
    
    list_init(&list);
    
    clock_start(c);
    
    for(i = 0; i < size; ++i) {
        
        if(list_empty(&list)) {
            list_insert(&list, &nodes[i].link);
        } else {
            list_for_each(&list, link) {
                tmp = container_of(link, struct list_node, link);
                
                if(nodes[i].val > tmp->val) {
                    list_insert(tmp->link.prev, &nodes[i].link);
                    break;
                }
            }
        }
    }
    
    fprintf(stdout, 
            "Elapsed time for %u sorted list insertions: %lu us.\n",
            size, clock_elapsed_us(c));
    
    link = list_front(&list);
    last = container_of(link, struct list_node, link);
    
    list_for_each(&list, link) {
        tmp = container_of(link, struct list_node, link);
        
        assert(last->val >= tmp->val);
        last = tmp;
    }
    
    clock_reset(c);
    
    if(!list_empty(&list)) {
        link = list_take_front(&list);
        last = container_of(link, struct list_node, link);
        
        while(!list_empty(&list)) {
            link = list_take_front(&list);
            tmp = container_of(link, struct list_node, link);
            assert(last->val >= tmp->val);
            
            last = tmp;
        }
    }
    
    fprintf(stdout, 
            "Elapsed time for %u sorted list removals: %lu us.\n",
            size, clock_elapsed_us(c));

    clock_delete(c);
    list_destroy(&list, NULL);
    free(nodes);
}

void test_heap_performance(int *data, unsigned int size)
{
    struct heap *heap;
    struct clock *c;
    unsigned int i, last;
    int err;
    
    heap = heap_new(0, &_int_compare);
    c    = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(heap);
    assert(c);
    
    clock_start(c);

    for(i = 0; i < size; ++i) {
        err = heap_insert(heap, (void *)(long) data[i]);
        assert(err == 0);
    }
    
    fprintf(stdout, 
            "Elapsed time for %u heap insertions: %lu us.\n",
            size, clock_elapsed_us(c));
    
    clock_reset(c);
    
    if(heap_size(heap)) {
        last = (long) heap_take(heap);
        
        while(!heap_empty(heap)) {
            i = (long) heap_take(heap);
            assert(i <= last);
            last = i;
        }
    }
    
    fprintf(stdout, 
            "Elapsed time for %u heap removals: %lu us.\n",
            size, clock_elapsed_us(c));
    
    clock_delete(c);
    heap_delete(heap);
}

void performance_comparison(int argc, char * const argv[])
{
    int fd, err, *data;
    unsigned int i, num;
    
    if(argc < 0)
        return;
    
    
    num = atoi(argv[1]);
    
    data = malloc(num * sizeof(*data));
    
    fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    
    for(i = 0; i < num; ++i) {
        err = read(fd, data + i, sizeof(*data));
        assert(err == sizeof(*data));
        data[i] &= INT_MAX;
    }
    
    close(fd);
    
    test_heap_performance(data, num);
    test_avltree_performance(data, num);
    test_sorted_list_performance(data, num);
    
    free(data);
}

void test_functionality(void)
{
    struct heap *heap;
    unsigned int i, num_elements, last;
    int fd, err, *data;

    
    num_elements = 1000;
    
    data = malloc(num_elements * sizeof(*data));
    assert(data);
    
    fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    
    for(i = 0; i < num_elements; ++i) {
        err = read(fd, data + i, sizeof(*data));
        assert(err == sizeof(*data));
        data[i] &= INT_MAX;
    }
    
    close(fd);
    
    heap = heap_new(0, &_int_compare);
    assert(heap);
    
    for(i = 0; i < num_elements; ++i) {
        err = heap_insert(heap, (void *)(long) data[i]);
        assert(err == 0);
    }

    heap_check(heap);
    
    if(heap_size(heap)) {
        last = (long) heap_take(heap);
        
        while(!heap_empty(heap)) {
            i    = (long) heap_take(heap);
            assert(i <= last);
            last = i;
        }
    }
    
    free(data);
    heap_delete(heap);
}

int main(int argc, char *argv[])
{
    test_functionality();
    performance_comparison(argc, argv);
    
    return EXIT_SUCCESS;
}