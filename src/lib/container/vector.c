
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "vector.h"

#define VECTOR_DEFAULT_SIZE 32

struct vector *vector_new(unsigned int size)
{
    struct vector *vec;
    int err;
    
    vec = malloc(sizeof(*vec));
    if(!vec)
        return NULL;
    
    err = vector_init(vec, size);
    if(err < 0) {
        free(vec);
        return NULL;
    }
    
    return vec;
}

void vector_delete(struct vector *__restrict vec, void (*data_delete)(void *))
{
    vector_destroy(vec, data_delete);
    free(vec);
}

int vector_init(struct vector *__restrict vec, unsigned int size)
{
    int capacity;
    
    capacity = VECTOR_DEFAULT_SIZE;
    
    while(capacity < size)
        capacity <<= 1;

    vec->_data = calloc(capacity, sizeof(*vec->_data));
    if(!vec->_data)
        return -errno;
    
    vec->_size = size;
    vec->_capacity = capacity;
    
    return 0;
}

void vector_destroy(struct vector *__restrict vec, void (*data_delete)(void *))
{
    vector_clear(vec, data_delete);
    free(vec->_data);
}

void vector_clear(struct vector *__restrict vec, void (*data_delete)(void *))
{
    if(!data_delete) {
        vec->_size = 0;
        return;
    }
    
    while(vec->_size--) {
        if(vec->_data[vec->_size]) {
            data_delete(vec->_data[vec->_size]);
            vec->_data[vec->_size] = NULL;
        }
    }
}

void vector_sort(struct vector *__restrict vec,
                 int (*data_compare)(const void *, const void *))
{
    qsort(vec->_data, vec->_size, sizeof(*vec->_data), data_compare);
}

inline unsigned int vector_size(const struct vector *__restrict vec)
{
    return vec->_size;
}

inline bool vector_empty(const struct vector *__restrict vec)
{
    return vec->_size == 0;
}

int vector_set_capacity(struct vector *__restrict vec, unsigned int capacity)
{
    void **data;
    
    data = realloc(vec->_data, capacity * sizeof(*data));
    if(!data)
        return -errno;
    
    if(vec->_size > capacity)
        vec->_size = capacity;
    
    vec->_data = data;
    vec->_capacity = capacity;
    
    return 0;
}

inline unsigned int vector_capacity(const struct vector *__restrict vec)
{
    return vec->_capacity;
}

int vector_squeeze(struct vector *__restrict vec)
{
    return vector_set_capacity(vec, vec->_size);
}

int vector_insert_back(struct vector *__restrict vec, void *data)
{
    int err;
    
    if(vec->_size >= vec->_capacity) {
        err = vector_set_capacity(vec, vec->_capacity << 1);
        if(err < 0)
            return err;
    }
    
    vec->_data[vec->_size] = data;
    vec->_size += 1;
    
    return 0;
}

inline void *vector_take_back(struct vector *__restrict vec)
{
    void *data;
    
    vec->_size -= 1;
    
    data = vec->_data[vec->_size];
    vec->_data[vec->_size] = NULL;
    
    return data;
}

inline void **vector_at(struct vector *__restrict vec, unsigned int i)
{
    return vec->_data + i;
}

inline void **vector_start(struct vector *__restrict vec)
{
    return vec->_data;
}

inline void **vector_end(struct vector *__restrict vec)
{
    return vec->_data + vec->_size - 1;
}
