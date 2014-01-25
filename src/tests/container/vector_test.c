#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <vector.h>
#include <clock.h>
#include <macro.h>

int compare(const void *a, const void *b)
{
    return (long) *(const int **) a - (long) *(const int **) b;
}

void print_vector(struct vector *__restrict v)
{
    void **data;
    int i;

    i = 0;
    
    vector_for_each(v, data)
        fprintf(stdout, "vec(%d) = %d\n", i++, (int)(long) *data);
        
    fprintf(stdout, "\n");
}

void test_sort_vector(void)
{
    struct vector *v;
    int a[] = { 13, 20, -7, 55, 42, 111, 76 };
    int i;
    
    v = vector_new(ARRAY_SIZE(a));
    assert(v);
        
    for(i = 0; i < ARRAY_SIZE(a); ++i)
        *vector_at(v, i) = (void *)(long) a[i];
    
    vector_sort(v, &compare);
    
    print_vector(v);
    
    for(i = 1; i < ARRAY_SIZE(a); ++i)
        assert((int)(long)*vector_at(v, i - 1) <= (int)(long)*vector_at(v, i));
    
    vector_delete(v, NULL);
}

void test_sort_large_vector(void)
{
#define N_ELEMENTS 10000000
    struct vector *v;
    struct clock *c;
    int i;
    void **data;
    
    v = vector_new(N_ELEMENTS);
    c = clock_new(CLOCK_MONOTONIC);
    assert(v);
    assert(c);
    
    i = 0;
    
    vector_for_each(v, data)
        *data = (void *)(long) N_ELEMENTS - i++;
        
    clock_start(c);
    
    vector_sort(v, &compare);
    
    clock_stop(c);
    
    fprintf(stdout, 
            "Elapsed sorting time (%u elements): %lu us.\n", 
            N_ELEMENTS, clock_elapsed_us(c));
    
    for(i = 1; i < N_ELEMENTS; ++i)
        assert((int)(long)*vector_at(v, i - 1) <= (int)(long)*vector_at(v, i));
    
    vector_delete(v, NULL);
    clock_delete(c);
}

void test_insert(void)
{
    struct vector *vec;
    int size = 10, i, err;
    
    vec = vector_new(10);
    assert(vec);
    
    for(i = 0; i < size; ++i)
        *vector_at(vec, i) = (void *)(long) i;
    
    err = vector_insert_front(vec, (void *) 1000);
    assert(err == 0);
    err = vector_insert_at(vec, size / 2, (void *)(long) size + 1);
    assert(err == 0);
    
    size = vector_size(vec);
    
    print_vector(vec);
    
    vector_delete(vec, NULL);
}

void test_take(void)
{
    struct vector *vec;
    int size = 10, i;
    void **data;
    
    vec = vector_new(10);
    assert(vec);
    
    i = 0;
    
    vector_for_each(vec, data)
        *data = (void *)(long) i++;

    assert((long)vector_take_at(vec, size / 2) == (size / 2));
    assert((long)vector_take_front(vec) == 0); 
    assert((long)vector_take_back(vec) == size - 1);
    
    assert(vector_size(vec) == size - 3);
    
    print_vector(vec);
    vector_delete(vec, NULL);
}

int main(int argc, char *argv[])
{
    test_sort_vector();
    test_sort_large_vector();
    test_insert();
    test_take();
    
    return EXIT_SUCCESS;
}