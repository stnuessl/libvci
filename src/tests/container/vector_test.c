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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include <libvci/vector.h>
#include <libvci/clock.h>
#include <libvci/macro.h>
#include <libvci/random.h>
#include <libvci/compare.h>

static void check_sorted_vector(struct vector *__restrict v)
{
    unsigned int i, size;
    
    size = vector_size(v);
    
    for(i = 1; i < size; ++i)
        assert((long)*vector_at(v, i - 1) <= (long)*vector_at(v, i));
}

void test_sort_vector(void)
{
    struct vector *v;
    int a[] = { 13, 20, -7, 55, 42, 111, 76 };
    int i;
    
    v = vector_new(ARRAY_SIZE(a));
    assert(v);
    
    vector_set_data_compare(v, &compare_int);
    
    /*
     * vector_at() doesn't work on vector with size 0,
     * so we artifically increase it.
     */
    for(i = 0; i < ARRAY_SIZE(a); ++i)
        vector_insert_back(v, (void *)(long) 1);
    
    for(i = 0; i < ARRAY_SIZE(a); ++i)
        *vector_at(v, i) = (void *)(long) a[i];
    
    vector_sort(v);
    
    for(i = 0; i < vector_size(v); ++i)
        fprintf(stdout, "%d\n", (int)(long)*vector_at(v, i));
    
    check_sorted_vector(v);
    
    vector_delete(v);
}

void test_sort_large_vector(void)
{
#define N_ELEMENTS 10000000
    struct vector *v;
    struct random *r;
    struct clock *c;
    int i;
    unsigned int rand;
    
    v = vector_new(N_ELEMENTS);
    c = clock_new(CLOCK_MONOTONIC);
    r = random_new();
    assert(v);
    assert(c);
    assert(r);
    
    vector_set_data_compare(v, &compare_uint);
    
    i = 0;
    
    for(i = 0; i < N_ELEMENTS; ++i) {
        rand = random_uint(r);
        assert(vector_insert_at(v, i, (void *)(long) rand) == 0);
    }
        
    clock_start(c);
    
    vector_sort(v);
    
    clock_stop(c);
    
    fprintf(stdout, 
            "Elapsed sorting time (%u elements): %lu ms.\n", 
            N_ELEMENTS, clock_elapsed_ms(c));
    
    check_sorted_vector(v);
    
    random_delete(r);
    clock_delete(c);
    vector_delete(v);
}

void test_insert(void)
{
    struct vector *vec;
    int size = 10, i, err;
    
    vec = vector_new(10);
    assert(vec);
    
    for(i = 0; i < size; ++i)
        vector_insert_back(vec, (void *)(long) i);
    
    err = vector_insert_front(vec, (void *) 1000);
    assert(err == 0);
    err = vector_insert_at(vec, size / 2, (void *)(long) size + 1);
    assert(err == 0);
    
    size = vector_size(vec);
        
    vector_delete(vec);
}

void test_take(void)
{
    struct vector *vec;
    int size = 10, i;
    
    vec = vector_new(size);
    assert(vec);
    
    for(i = 0; i < size; ++i)
        vector_insert_back(vec, (void *)(long) i);

    assert((long)vector_take_at(vec, size / 2) == (size / 2));
    assert((long)vector_take_front(vec) == 0); 
    assert((long)vector_take_back(vec) == size - 1);
    
    assert(vector_size(vec) == size - 3);
    
    vector_delete(vec);
}

void test_sorted(void)
{
    struct vector *vec;
    struct random *rand;
    int size, i, err, tmp, **p;
    
    size = (int) 10000;
    
    vec = vector_new(0);
    rand = random_new();
    assert(vec);
    assert(rand);
    
    vector_set_data_compare(vec, &compare_int);
    
    for(i = 0; i < size; ++i) {
        tmp = random_uint(rand);
        err = vector_insert_sorted(vec, (void *)(long) tmp);
        assert(err == 0);
    }
    
    vector_set_capacity(vec, min(vector_size(vec), 10));
    
    vector_take(vec, *vector_front(vec));
    vector_take(vec, *vector_back(vec));
        
    vector_for_each(vec, p)
        fprintf(stdout, "%u\n", (unsigned int)(long) *p);
    
    random_delete(rand);
    vector_delete(vec);
}

void test_sorted_simple(void)
{
    struct vector *v;
    int i, **p, err, a[] = { 0, 1, 2 ,3 ,4 ,5 ,6 ,7 ,8, 9 };
    
    v = vector_new(0);
    assert(v);
    
    vector_set_data_compare(v, &compare_int);
    
    for(i = 0; i < ARRAY_SIZE(a); ++i) {
        err = vector_insert_sorted(v, (void *)(long) a[i]);
        assert(err == 0);
    }
    
    vector_insert_front(v, (void *) -1);
    vector_insert_back(v, (void *) 10);
    
    assert((int)(long) *vector_front(v) == -1);
    assert((int)(long) *vector_back(v) == 10);
    
    vector_take(v, *vector_front(v));
    vector_take(v, *vector_back(v));
    vector_take_sorted(v, (void *) 4);
    vector_take(v, (void *) 5);
    
    vector_for_each(v, p)
        assert((int)(long) *p != 4 && (int)(long) *p != 5);
    
    vector_delete(v);
}

int main(int argc, char *argv[])
{
    test_sort_vector();
    test_sort_large_vector();
    test_insert();
    test_take();
    test_sorted();
    test_sorted_simple();
    
    return EXIT_SUCCESS;
}