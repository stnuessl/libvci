#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "container_util.h"
#include "buffer.h"

#define BUFFER_DEFAULT_SIZE 128


static int _buffer_resize(struct buffer *__restrict buf, size_t new_size)
{
    void *new_data;
    
    new_size = adjust(new_size, BUFFER_DEFAULT_SIZE);
    
    if(new_size == buf->size)
        return 0;
    
    new_data = realloc(buf->data, new_size);
    if(!new_data)
        return -errno;
    
    buf->data = new_data;
    buf->size = new_size;
    
    return 0;
}


static void _buffer_remove(struct buffer *__restrict buf, 
                           size_t i, 
                           size_t size)
{
    memmove(buf->data + i, buf->data + i + size, buf->used - i - size + 1);
    
    buf->used -= size;
}

struct buffer *buffer_new(size_t size)
{
    struct buffer *buf;
    
    buf = malloc(sizeof(*buf));
    if(!buf)
        return NULL;

    if(buffer_init(buf, size) < 0) {
        free(buf);
        return NULL;
    }
    
    return buf;
}

struct buffer *buffer_clone(const struct buffer *__restrict buf, 
                            bool clear_accessed)
{
    struct buffer *clone;
    
    clone = buffer_new(buf->used);
    if(!clone)
        return NULL;
    
    memcpy(clone->data, buf->data, buf->used);
    
    clone->used = buf->used;
    
    if(!clear_accessed)
        clone->accessed = buf->accessed;
    
    return clone;
}

void buffer_delete(struct buffer *__restrict buf)
{
    buffer_destroy(buf);
    free(buf);
}

int buffer_init(struct buffer *__restrict buf, size_t size)
{
    size = adjust(size, BUFFER_DEFAULT_SIZE);
    
    memset(buf, 0, sizeof(*buf));
    
    buf->data = malloc(size);
    if(!buf->data)
        return -errno;
    
    buf->size = size;
    
    return 0;
}

void buffer_destroy(struct buffer *__restrict buf)
{
    free(buf->data);
}

void buffer_clear(struct buffer *__restrict buf)
{
    buf->accessed = 0;
    buf->used     = 0;
}

int buffer_prepare_write(struct buffer *__restrict buf, size_t size)
{
    size_t new_size;
    
    if(buf->used + size <= buf->size)
        return 0;
    
    for(new_size = buf->size; new_size <= buf->size + size; new_size <<= 1);
    
    return _buffer_resize(buf, new_size);
}

void buffer_write(struct buffer *__restrict buf, 
                  const void *__restrict data,
                  size_t data_size)
{
    memcpy(buf->data + buf->used, data, data_size);
    
    buf->used += data_size;
}

#define BUFFER_DEFINE_WRITE(type)                                              \
                                                                               \
void buffer_write_##type(struct buffer *__restrict buf, type data)             \
{                                                                              \
    buffer_write(buf, &data, sizeof(data));                                    \
}

BUFFER_DEFINE_WRITE(char)
BUFFER_DEFINE_WRITE(short)
BUFFER_DEFINE_WRITE(int)
BUFFER_DEFINE_WRITE(long)
BUFFER_DEFINE_WRITE(float)
BUFFER_DEFINE_WRITE(double)

#undef BUFFER_DEFINE_WRITE

void buffer_read(struct buffer *__restrict buf, 
                 void *__restrict data,
                 size_t data_size)
{
    memcpy(data, buf->data + buf->accessed, data_size);
    
    buf->accessed += data_size;
}

#define BUFFER_DEFINE_READ(type)                                               \
                                                                               \
type buffer_read_##type(struct buffer *__restrict buf)                         \
{                                                                              \
    type data;                                                                 \
                                                                               \
    buffer_read(buf, &data, sizeof(data));                                     \
                                                                               \
    return data;                                                               \
}

BUFFER_DEFINE_READ(char)
BUFFER_DEFINE_READ(short)
BUFFER_DEFINE_READ(int)
BUFFER_DEFINE_READ(long)
BUFFER_DEFINE_READ(float)
BUFFER_DEFINE_READ(double)

#undef BUFFER_DEFINE_READ

size_t buffer_bytes_accessible(const struct buffer *__restrict buf)
{
    return buf->used - buf->accessed;
}

void buffer_squeeze(struct buffer *__restrict buf)
{
    _buffer_resize(buf, buf->used);
}

void buffer_clear_accessed(struct buffer *__restrict buf)
{
    _buffer_remove(buf, 0, buf->accessed);
    
    buf->accessed = 0;
}

void buffer_clear_unaccessed(struct buffer *__restrict buf)
{
    _buffer_remove(buf, buf->accessed, buf->used - buf->accessed);
}

inline void *buffer_data(struct buffer *__restrict buf)
{
    return buf->data;
}

inline size_t buffer_size(const struct buffer *__restrict buf)
{
    return buf->used;
}

inline bool buffer_empty(const struct buffer *__restrict buf)
{
    return buffer_size(buf) == 0;
}

inline char buffer_at(const struct buffer *__restrict buf, int i)
{
    return *(char *) (buf->data + i);
}