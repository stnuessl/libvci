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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libvci/queue.h>
#include <libvci/clock.h>
#include <libvci/macro.h>

struct data {
    unsigned int data;
    struct link link;
};

void init_data(struct data *data, unsigned int size)
{
    while(size--)
        data[size].data = size;
}

void test_functionality(void)
{
#define QUEUE_SIZE 10
    struct queue *q;
    struct data data[QUEUE_SIZE], *tmp;
    int i;
    
    q = queue_new();
    assert(q);
    
    init_data(data, ARRAY_SIZE(data));
    
    for(i = 0; i < ARRAY_SIZE(data); ++i)
        queue_insert(q, &data[i].link);
    
    while(!queue_empty(q)) {
        tmp = container_of(queue_take(q), struct data, link);
        fprintf(stdout, "%u ", tmp->data);
    }
    
    fprintf(stdout, "\n");
    
    queue_delete(q, NULL);
#undef QUEUE_SIZE
}



void test_performance(void)
{
#define QUEUE_SIZE 1000000
    static struct data data[QUEUE_SIZE];
    struct clock *c;
    struct queue *q;

    int i;

    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    q = queue_new();
    assert(c);
    assert(q);
    
    clock_start(c);
    
    for(i = 0; i < QUEUE_SIZE; ++i)
        queue_insert(q, &data[i].link);
    
    while(!queue_empty(q))
        assert(container_of(queue_take(q), struct data, link));
    
    fprintf(stdout, "Time elapsed for %u elements: %lu us\n",
            QUEUE_SIZE, clock_elapsed_us(c));
    
    clock_delete(c);
    queue_delete(q, NULL);
#undef QUEUE_SIZE
}

int main(int argc, char *argv[])
{
    test_functionality();
    test_performance();
    
    return EXIT_SUCCESS;
}