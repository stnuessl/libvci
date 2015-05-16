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
#include <unistd.h>
#include <assert.h>

#include <libvci/list.h>
#include <libvci/clock.h>
#include <libvci/macro.h>

struct data {
    int a;
    struct link link;
};

void data_delete_from_link(struct link *link)
{
    free(container_of(link, struct data, link));
}

void list_test_usage(void)
{
    struct link list, *tmp;
    struct data *data;
    struct clock *c;
    int i, num;
    
    num = 10000000;
    
    list_init(&list);
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(c);
    
    for(i = 0; i < num; ++i) {
        data = malloc(sizeof(*data));
        
        data->a = i;
        list_insert(&list, &data->link);
    }
    
    clock_start(c);
    
    list_for_each(&list, tmp) {
        data = container_of(tmp, struct data, link);
        
        if(data->a == 0)
            fprintf(stdout,
                    "Accessed last element (of %d) in list within %lu ms.\n",
                    num, clock_elapsed_ms(c));
    }
    
    clock_delete(c);
    list_destroy(&list, &data_delete_from_link);
}

int main(int argc, char *argv[])
{
    
    (void) argc;
    (void) argv;
    
    struct link list, *link, *tmp;
    struct data a = {
        .a = 10
    }, b = {
        .a = 11 
    }, c = {
        .a = 12
    }, *d;
    
    list_init(&list);
    
    list_insert(&list, &a.link);
    list_insert(&list, &b.link);
    list_insert(&list, &c.link);
    
    printf("List iteration forward.\n");
    
    list_for_each(&list, link) {
        d = container_of(link, struct data, link);
        
        printf("value: %d\n", d->a);
    }
    
    printf("List iteration backward.\n");
    
    list_for_each_reverse_safe(&list, link, tmp) {
        d = container_of(link, struct data, link);
        
        printf("value: %d\n", d->a);
        link = NULL;
    }
    
    list_test_usage();
   
    return EXIT_SUCCESS;
}