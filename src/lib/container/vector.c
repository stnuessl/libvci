#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "container_p.h"
#include "vector.h"

#define VECTOR_DEFAULT_CAPACITY 8

struct vector *vector_new(unsigned int capacity)
{
    struct vector *vec;
    int err;
    
    vec = malloc(sizeof(*vec));
    if(!vec)
        return NULL;
    
    err = vector_init(vec, capacity);
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

int vector_init(struct vector *__restrict vec, unsigned int capacity)
{
    capacity = adjust(capacity, VECTOR_DEFAULT_CAPACITY);
    
    vec->data = calloc(capacity, sizeof(*vec->data));
    if(!vec->data)
        return -errno;

    vec->size = 0;
    vec->capacity = capacity;
    
    return 0;
}

void vector_destroy(struct vector *__restrict vec, void (*data_delete)(void *))
{
    vector_clear(vec, data_delete);
    free(vec->data);
}

void vector_clear(struct vector *__restrict vec, void (*data_delete)(void *))
{
    if(!data_delete) {
        vec->size = 0;
        return;
    }
    
    while(vec->size--) {
        if(vec->data[vec->size]) {
            data_delete(vec->data[vec->size]);
            vec->data[vec->size] = NULL;
        }
    }
}

void vector_sort(struct vector *__restrict vec,
                 int (*data_compare)(const void *, const void *))
{
    qsort(vec->data, vec->size, sizeof(*vec->data), data_compare);
}

inline unsigned int vector_size(const struct vector *__restrict vec)
{
    return vec->size;
}

inline bool vector_empty(const struct vector *__restrict vec)
{
    return vec->size == 0;
}

int vector_set_capacity(struct vector *__restrict vec, unsigned int capacity)
{
    void **data;
    
    capacity = adjust(capacity, VECTOR_DEFAULT_CAPACITY);
    
    if(capacity == vec->capacity)
        return 0;
    
    data = realloc(vec->data, capacity * sizeof(*data));
    if(!data)
        return -errno;
    
    if(vec->size > capacity)
        vec->size = capacity;
    
    vec->data = data;
    vec->capacity = capacity;
    
    return 0;
}

inline unsigned int vector_capacity(const struct vector *__restrict vec)
{
    return vec->capacity;
}

int vector_squeeze(struct vector *__restrict vec)
{
    return vector_set_capacity(vec, vec->size);
}

int vector_insert_front(struct vector *__restrict vec, void *data)
{
    return vector_insert_at(vec, 0, data);
}

int vector_insert_at(struct vector *__restrict vec, unsigned int i, void *data)
{
    size_t move_size;
    int err;
    
    if(vec->size >= vec->capacity) {
        err = vector_set_capacity(vec, vec->capacity << 1);
        if(err < 0)
            return err;
    }
    
    move_size = (vec->size - i) * sizeof(*vec->data);
    
    memmove(vec->data + i + 1, vec->data + i, move_size);
    
    vec->data[i] = data;
    vec->size += 1;
    
    return 0;
}

int vector_insert_back(struct vector *__restrict vec, void *data)
{
    int err;
    
    if(vec->size >= vec->capacity) {
        err = vector_set_capacity(vec, vec->capacity << 1);
        if(err < 0)
            return err;
    }
    
    vec->data[vec->size] = data;
    vec->size += 1;
    
    return 0;
}

void *vector_take_front(struct vector *__restrict vec)
{
    return vector_take_at(vec, 0);
}

void *vector_take_at(struct vector *__restrict vec, unsigned int i)
{
    void *data;
    size_t move_size;
    
    data = vec->data[i];
    
    move_size = (vec->size - i) * sizeof(*vec->data);
    vec->size -= 1;
    
    memmove(vec->data + i, vec->data + i + 1, move_size);
    
    return data;
}

void *vector_take_back(struct vector *__restrict vec)
{
    vec->size -= 1;
    
    return vec->data[vec->size];
}

void **vector_at(struct vector *__restrict vec, unsigned int i)
{
    return vec->data + i;
}

void **vector_start(struct vector *__restrict vec)
{
    return vec->data;
}

void **vector_end(struct vector *__restrict vec)
{
    return vec->data + vec->size - 1;
}
