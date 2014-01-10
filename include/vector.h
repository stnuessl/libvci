#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdbool.h>

struct vector {
    void **data;
    unsigned int size;
    unsigned int capacity;
};


struct vector *vector_new(unsigned int capacity);

void vector_delete(struct vector *__restrict vec, void (*data_delete)(void *));

int vector_init(struct vector *__restrict vec, unsigned int capacity);

void vector_destroy(struct vector *__restrict vec, void (*data_delete)(void *));

void vector_clear(struct vector *__restrict vec, void (*data_delete)(void *));

void vector_sort(struct vector *__restrict vec,
                 int (*data_compare)(const void *, const void *));

inline unsigned int vector_size(const struct vector *__restrict vec);

inline bool vector_empty(const struct vector *__restrict vec);

int vector_set_capacity(struct vector *__restrict vec, unsigned int size);

inline unsigned int vector_capacity(const struct vector *__restrict vec);

int vector_squeeze(struct vector *__restrict vec);

int vector_insert_front(struct vector *__restrict vec, void *data);

int vector_insert_at(struct vector *__restrict vec, unsigned int i, void *data);

int vector_insert_back(struct vector *__restrict vec, void *data);

void *vector_take_front(struct vector *__restrict vec);

void *vector_take_at(struct vector *__restrict vec, unsigned int i);

void *vector_take_back(struct vector *__restrict vec);

void **vector_at(struct vector *__restrict vec, unsigned int i);

void **vector_start(struct vector *__restrict vec);

void **vector_end(struct vector *__restrict vec);

#define vector_for_each(vec, tmp)                                              \
    for((tmp) = (__typeof(tmp)) (vec)->data;                                   \
        (tmp) < (__typeof(tmp)) (vec)->data + (vec)->size;                     \
        ++(tmp))

#endif /* _VECTOR_H_ */