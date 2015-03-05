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

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "clock.h"

#define LOG_DATE          (1 << 0)
#define LOG_TIMESTAMP     (1 << 1)
#define LOG_PID           (1 << 2)
#define LOG_TAG           (1 << 3)
#define LOG_HOSTNAME      (1 << 4)
#define LOG_LEVEL         (1 << 5)

#define LOG_ALL                                                                \
    (LOG_DATE | LOG_TIMESTAMP | LOG_PID | LOG_TAG | LOG_HOSTNAME | LOG_LEVEL)

#define LOG_DEBUG    0
#define LOG_INFO     1
#define LOG_WARNING  2
#define LOG_ERROR    3

struct log {
    char *hostname;
    FILE *file;
    struct clock clock;
    
    uint8_t flags;
    uint8_t level;
};

struct log *log_new(const char *__restrict path, uint8_t flags);

void log_delete(struct log *__restrict l);

int log_init(struct log *__restrict l,
             const char *__restrict path,
             uint8_t flags);

void log_destroy(struct log *__restrict l);

void log_set_file(struct log *__restrict l, FILE *f);

int log_fd(const struct log *__restrict l);

void log_set_level(struct log *__restrict l, uint8_t level);

int log_level(const struct log *__restrict l);

void log_append(struct log *__restrict l, const char *fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_printf(struct log *__restrict l, 
                uint8_t level,
                const char *__restrict tag,
                const char *__restrict fmt, 
                ...) __attribute__((format(printf,4,5)));

void log_vprintf(struct log *__restrict l,
                 uint8_t level,
                 const char *__restrict tag,
                 const char *__restrict fmt,
                 va_list vargs);

void log_debug(struct log *__restrict l, 
               const char *__restrict tag,
               const char *__restrict fmt,
               ...) __attribute__((format(printf,3,4)));

void log_info(struct log *__restrict l, 
              const char *__restrict tag,
              const char *__restrict fmt,
              ...) __attribute__((format(printf,3,4)));

void log_warning(struct log *__restrict l, 
                 const char *__restrict tag,
                 const char *__restrict fmt,
                 ...) __attribute__((format(printf,3,4)));

void log_error(struct log *__restrict l, 
               const char *__restrict tag,
               const char *__restrict fmt,
               ...)  __attribute__((format(printf,3,4)));

void log_clear(struct log *__restrict l);

void log_print(struct log *__restrict l, int fd);

#endif /* _LOG_H_ */