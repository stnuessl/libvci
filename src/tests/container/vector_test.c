

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <vector.h>
#include <clock.h>
#include <macros.h>

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

void sort_vector(void)
{
    struct vector *v;
    int a[] = { 13, 20, -7, 55, 42, 111, 76 };
    int i;
    
    v = vector_new(ARRAY_SIZE(a));
    assert(v);
    
//     for(i = 0; i < ARRAY_SIZE(a); ++i) {
//         err = vector_insert_back(v, (void *)(long) a[i]);
//         assert(err == 0);
//     }
    
    for(i = 0; i < ARRAY_SIZE(a); ++i)
        *vector_at(v, i) = (void *)(long) a[i];
    
    vector_sort(v, &compare);
    
    print_vector(v);
    
    for(i = 1; i < ARRAY_SIZE(a); ++i)
        assert((int)(long)*vector_at(v, i - 1) <= (int)(long)*vector_at(v, i));
    
    vector_delete(v, NULL);
}

void sort_large_vector(void)
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
    
    fprintf(stdout, "Elapsed time: %lu us.\n", clock_elapsed_us(c));
    
    for(i = 1; i < N_ELEMENTS; ++i)
        assert((int)(long)*vector_at(v, i - 1) <= (int)(long)*vector_at(v, i));
    
    vector_delete(v, NULL);
    clock_delete(c);
}

int main(int argc, char *argv[])
{
    struct vector *v;
    unsigned int size;
    int i;

    
    if(argc != 2) {
        fprintf(stderr, "*WARNING*: missing argument.\n"
                        "Usage: %s <size of vector>.\n",
                        argv[0]);
        return EXIT_FAILURE;
    }
    
    size = atoi(argv[1]);
    
    v = vector_new(size);
    assert(v);
    
    for(i = 0; i < size; ++i)
        *vector_at(v, i) = (void *)(long) size - i;
    
    print_vector(v);
    
    fprintf(stdout, "First: %d | Last: %d\n", 
            (int)(long)*vector_start(v), 
            (int)(long)*vector_end(v));
    
    assert((int)(long)vector_take_back(v) == 1);
    assert((int)(long)vector_take_back(v) == 2);
    
    vector_sort(v, &compare);
    
    vector_set_capacity(v, vector_size(v) - 2);

    print_vector(v);
    
    vector_clear(v, NULL);
    assert(vector_empty(v));
    assert(vector_capacity(v) > 0);

    vector_set_capacity(v, 128);
    
    assert(vector_capacity(v) == 128);
    
    vector_delete(v, NULL);
    
    sort_vector();
    sort_large_vector();
    
    return EXIT_SUCCESS;
}