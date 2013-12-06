#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdlib.h>
#include <stdbool.h>

struct buffer {
    void *data;
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