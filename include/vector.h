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

int vector_insert_back(struct vector *__restrict vec, void *data);

inline void *vector_take_back(struct vector *__restrict vec);

inline void **vector_at(struct vector *__restrict vec, unsigned int i);

inline void **vector_start(struct vector *__restrict vec);

inline void **vector_end(struct vector *__restrict vec);

#define vector_for_each(vec, data)                                             \
    for((data) = (vec)->data; (data) < (vec)->data + (vec)->size; ++(data))

#endif /* _VECTOR_H_ */