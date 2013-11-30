

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <vector.h>
#include <macros.h>

int compare(const void *a, const void *b)
{
    return *(const int **)a - *(const int **)b;
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

int main(int argc, char *argv[])
{
    struct vector *v;
    unsigned int size;
    int err, i;
    int a[] = { 13, 20, -7, 55, 42, 111, 76 };
    
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
        
    vector_sort(v, &compare);

    print_vector(v);
    
    vector_clear(v, NULL);
    
    for(i = 0; i < ARRAY_SIZE(a); ++i) {
        err = vector_insert_back(v, (void *)(long) a[i]);
        assert(err == 0);
    }
    
    vector_sort(v, &compare);
    
    print_vector(v);
    
    vector_delete(v, NULL);
    
    return EXIT_SUCCESS;
}