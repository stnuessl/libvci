#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "buffer.h"

#define BUFFER_STANDARD_SIZE 128

#define _is_power_of_two(val)    (val && !(val & (val - 1)))

static size_t _next_power_of_two(size_t val)
{
    int i;
    
    --val;
    
    for(i = 1; i <= (sizeof(val) >> 1); i <<= 1)
        val |= val >> i;
    
    return ++val;
}

static int _buffer_resize(struct buffer *__restrict buf, size_t new_size)
{
    void *new_data;
    
    if(!_is_power_of_two(new_size))
        new_size = _next_power_of_two(new_size);
    
    if(new_size == buf->_size)
        return 0;
    
    new_data = realloc(buf->_data, new_size);
    if(!new_data)
        return -errno;
    
    buf->_data = new_data;
    buf->_size = new_size;
    
    return 0;
}


static void _buffer_remove(struct buffer *__restrict buf, 
                           size_t i, 
                           size_t size)
{
    memmove(buf->_data + i, buf->_data + i + size, buf->_used - i - size + 1);
    
    buf->_used -= size;
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
    
    clone = buffer_new(buf->_used);
    if(!clone)
        return NULL;
    
    memcpy(clone->_data, buf->_data, buf->_used);
    
    clone->_used = buf->_used;
    
    if(!clear_accessed)
        clone->_accessed = buf->_accessed;
    
    return clone;
}

void buffer_delete(struct buffer *__restrict buf)
{
    buffer_destroy(buf);
    free(buf);
}

int buffer_init(struct buffer *__restrict buf, size_t size)
{
    if(size <= 0)
        size = BUFFER_STANDARD_SIZE;
    else if(!_is_power_of_two(size))
        size = _next_power_of_two(size);
    
    memset(buf, 0, sizeof(*buf));
    
    buf->_data = malloc(size);
    if(!buf->_data)
        return -errno;
    
    buf->_size = size;
    
    return 0;
}

void buffer_destroy(struct buffer *__restrict buf)
{
    free(buf->_data);
}

void buffer_clear(struct buffer *__restrict buf)
{
    buf->_accessed = 0;
    buf->_used     = 0;
}

int buffer_prepare_write(struct buffer *__restrict buf, size_t size)
{
    size_t new_size;
    
    if(buf->_used + size <= buf->_size)
        return 0;
    
    for(new_size = buf->_size; new_size <= buf->_size + size; new_size <<= 1);
    
    return _buffer_resize(buf, new_size);
}

void buffer_write(struct buffer *__restrict buf, 
                  const void *__restrict data,
                  size_t data_size)
{
    memcpy(buf->_data + buf->_used, data, data_size);
    
    buf->_used += data_size;
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
    memcpy(data, buf->_data + buf->_accessed, data_size);
    
    buf->_accessed += data_size;
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
    return buf->_used - buf->_accessed;
}

void buffer_squeeze(struct buffer *__restrict buf)
{
    _buffer_resize(buf, buf->_used);
}

void buffer_clear_accessed(struct buffer *__restrict buf)
{
    _buffer_remove(buf, 0, buf->_accessed);
    
    buf->_accessed = 0;
}

void buffer_clear_unaccessed(struct buffer *__restrict buf)
{
    _buffer_remove(buf, buf->_accessed, buf->_used - buf->_accessed);
}

inline void *buffer_data(struct buffer *__restrict buf)
{
    return buf->_data;
}

inline size_t buffer_size(const struct buffer *__restrict buf)
{
    return buf->_used;
}

inline bool buffer_empty(const struct buffer *__restrict buf)
{
    return buffer_size(buf) == 0;
}

inline char buffer_at(const struct buffer *__restrict buf, int i)
{
    return *(char *) (buf->_data + i);
}