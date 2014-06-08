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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdbool.h>

struct vector {
    int (*data_compare)(const void *, const void *);
    void (*data_delete)(void *);
    
    void **data;
    
    unsigned int size;
    unsigned int capacity;
};


struct vector *vector_new(unsigned int capacity);

void vector_delete(struct vector *__restrict vec);

int vector_init(struct vector *__restrict vec, unsigned int capacity);

void vector_destroy(struct vector *__restrict vec);

void vector_clear(struct vector *__restrict vec);

inline unsigned int vector_size(const struct vector *__restrict vec);

inline bool vector_empty(const struct vector *__restrict vec);

int vector_set_capacity(struct vector *__restrict vec, unsigned int capacity);

inline unsigned int vector_capacity(const struct vector *__restrict vec);

int vector_squeeze(struct vector *__restrict vec);

int vector_insert_front(struct vector *__restrict vec, void *data);

int vector_insert_at(struct vector *__restrict vec, unsigned int i, void *data);

int vector_insert_back(struct vector *__restrict vec, void *data);

void *vector_take_front(struct vector *__restrict vec);

void *vector_take_at(struct vector *__restrict vec, unsigned int i);

void *vector_take_back(struct vector *__restrict vec);

void *vector_take(struct vector *__restrict vec, void *data);

void vector_take_all(struct vector *__restrict vec, void *data);

void **vector_at(struct vector *__restrict vec, unsigned int i);

void **vector_front(struct vector *__restrict vec);

void **vector_back(struct vector *__restrict vec);

void vector_sort(struct vector *__restrict vec);

int vector_insert_sorted(struct vector *__restrict vec, void *data);

void *vector_take_sorted(struct vector *__restrict vec, void *data);

/* 
 * This data compare function must absolutely make sure that it compares
 * the corresponding types and not void * -values
 */
void vector_set_data_compare(struct vector *__restrict vec, 
                             int (*data_compare)(const void *, const void *));

int (*vector_data_compare(struct vector *__restrict vec))
                          (const void *, const void *);

void vector_set_data_delete(struct vector *__restrict vec, 
                            void (*data_delete)(void *));

void (*vector_data_delete(struct vector *__restrict vec))(void *);

#define vector_for_each(vec, tmp)                                              \
    for((tmp) = (__typeof(tmp)) (vec)->data;                                   \
        (tmp) < (__typeof(tmp)) (vec)->data + (vec)->size;                     \
        ++(tmp))

#endif /* _VECTOR_H_ */