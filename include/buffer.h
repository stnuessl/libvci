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

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdlib.h>
#include <stdbool.h>

struct buffer {
    char *data;
    size_t accessed;
    size_t used;
    size_t size;
};


struct buffer *buffer_new(size_t size);

struct buffer *buffer_clone(const struct buffer *__restrict buf, 
                            bool clear_accessed);

void buffer_delete(struct buffer *__restrict buf);

int buffer_init(struct buffer *__restrict buf, size_t size);

void buffer_destroy(struct buffer *__restrict buf);

void buffer_clear(struct buffer *__restrict buf);

int buffer_prepare_write(struct buffer *__restrict buf, size_t size);

void buffer_write(struct buffer *__restrict buf, 
                  const void *__restrict data,
                  size_t data_size);

#define BUFFER_DEFINE_WRITE(type)                                              \
                                                                               \
void buffer_write_##type(struct buffer *__restrict buf, type data);

BUFFER_DEFINE_WRITE(char)
BUFFER_DEFINE_WRITE(short)
BUFFER_DEFINE_WRITE(int)
BUFFER_DEFINE_WRITE(long)
BUFFER_DEFINE_WRITE(float)
BUFFER_DEFINE_WRITE(double)

#undef BUFFER_DEFINE_WRITE

void buffer_read(struct buffer *__restrict buf, 
                 void *__restrict data,
                 size_t data_size);

#define BUFFER_DEFINE_READ(type)                                               \
                                                                               \
type buffer_read_##type(struct buffer *__restrict buf);

BUFFER_DEFINE_READ(char)
BUFFER_DEFINE_READ(short)
BUFFER_DEFINE_READ(int)
BUFFER_DEFINE_READ(long)
BUFFER_DEFINE_READ(float)
BUFFER_DEFINE_READ(double)

#undef BUFFER_DEFINE_READ

size_t buffer_bytes_accessible(const struct buffer *__restrict buf);

void buffer_squeeze(struct buffer *__restrict buf);

void buffer_clear_accessed(struct buffer *__restrict buf);

void buffer_clear_unaccessed(struct buffer *__restrict buf);

inline void *buffer_data(struct buffer *__restrict buf);

inline size_t buffer_size(const struct buffer *__restrict buf);

inline bool buffer_empty(const struct buffer *__restrict buf);

inline char buffer_at(const struct buffer *__restrict buf, int i);

#endif /* _BUFFER_H_ */