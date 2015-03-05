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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdbool.h>

#include "clock.h"
#include "log.h"

static const char *log_level_string(int severity)
{
    static const char *strings[] = {
        [LOG_DEBUG]   = "DEBUG",
        [LOG_INFO]    = "INFO",
        [LOG_WARNING] = "WARNING",
        [LOG_ERROR]   = "ERROR"
    };
    
    return strings[severity];
}

static void write_line_header(struct log *__restrict l, 
                              uint8_t level,
                              const char *__restrict tag)
{
    time_t now;
    struct tm ltime;
    unsigned long elapsed;
    
    if(l->flags & LOG_DATE) {
        now = time(NULL);
        
        localtime_r(&now, &ltime);
        
        fprintf(l->file, "%04d-%02d-%02d :: %02d:%02d:%02d ",
                ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday,
                ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
    }
    
    if(l->flags & LOG_TIMESTAMP) {
        elapsed = clock_elapsed_us(&l->clock);
        fprintf(l->file, "| %*lf ", 11, (double) elapsed / 1e6);
    }
    
    if(l->flags & LOG_HOSTNAME)
        fprintf(l->file, "| %s ", l->hostname);
    
    if(l->flags & LOG_PID)
        fprintf(l->file, "| %*u ", 5, getpid());
    
    if(l->flags & LOG_LEVEL)
        fprintf(l->file, "| %*s ", 8, log_level_string(level));
    
    fprintf(l->file, "| <> ");
    
    if(l->flags & LOG_TAG)
        fprintf(l->file,  "%s ", tag);
    
    fprintf(l->file, ": ");
}


static void print(struct log *__restrict l,
                  uint8_t level,
                  const char *__restrict tag,
                  const char *__restrict fmt,
                  va_list vargs)
{
    if(level < l->level)
        return;
    
    write_line_header(l, level, tag);
    vfprintf(l->file, fmt, vargs);
    fflush(l->file);
}


struct log *log_new(const char *__restrict path, uint8_t flags)
{
    struct log *l;
    int err;
    
    l = malloc(sizeof(*l));
    if(!l)
        return NULL;
    
    err = log_init(l, path, flags);
    if(err < 0) {
        free(l);
        return NULL;
    }
    
    return l;
}

void log_delete(struct log *__restrict l)
{
    log_destroy(l);
    free(l);
}

int log_init(struct log *__restrict l,
             const char *__restrict path,
             uint8_t flags)
{
#define HOSTNAME_SIZE 64
    char hostname[HOSTNAME_SIZE];
    int err;
    
    
    l->file = fopen(path, "a+");
    if(!l->file) {
        err = -errno;
        goto out;
    }
    
    if(flags & LOG_TIMESTAMP) {
        err = clock_init(&l->clock, CLOCK_MONOTONIC);
        if(err < 0)
            goto cleanup1;
        
        clock_start(&l->clock);
    }
    
    if(flags & LOG_HOSTNAME) {
        hostname[HOSTNAME_SIZE - 1] = '\0';
        
        err = gethostname(hostname, HOSTNAME_SIZE - 1);
        if(err < 0)
            goto cleanup2;
        
        l->hostname = strdup(hostname);
        if(!l->hostname) {
            err = -errno;
            goto cleanup2;
        }
    }

    l->flags = flags;
    l->level = LOG_INFO;
    
    fprintf(l->file, "--\n");
    
    return 0;

cleanup2:
    if(flags & LOG_TIMESTAMP)
        clock_destroy(&l->clock);
cleanup1:
    fclose(l->file);
out:
    return err;
}

void log_destroy(struct log *__restrict l)
{
    if(l->flags & LOG_TIMESTAMP)
        clock_destroy(&l->clock);
    
    if(l->flags & LOG_HOSTNAME)
        free(l->hostname);
        
    fclose(l->file);
}

void log_set_file(struct log *__restrict l, FILE *f)
{
    if(l->file)
        fclose(l->file);

    l->file = f;
}

int log_fd(const struct log *__restrict l)
{
    return fileno(l->file);
}

void log_set_level(struct log *__restrict l, uint8_t level)
{
    l->level = level;
}

int log_level(const struct log *__restrict l)
{
    return l->level;
}

void log_append(struct log *__restrict l, const char *fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    vfprintf(l->file, fmt, vargs);
    va_end(vargs);
}

void log_printf(struct log *__restrict l, 
               uint8_t level,
               const char *__restrict tag,
               const char *__restrict fmt, 
               ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    print(l, level, tag, fmt, vargs);
    va_end(vargs);
}

void log_vprintf(struct log *__restrict l,
                 uint8_t level,
                 const char *__restrict tag,
                 const char *__restrict fmt,
                 va_list vargs)
{
    print(l, level, tag, fmt, vargs);
}

void log_debug(struct log *__restrict l, 
               const char *__restrict tag,
               const char *__restrict fmt,
               ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    print(l, LOG_DEBUG, tag, fmt, vargs);
    va_end(vargs);
}

void log_info(struct log *__restrict l, 
              const char *__restrict tag,
              const char *__restrict fmt,
              ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    print(l, LOG_INFO, tag, fmt, vargs);
    va_end(vargs);
}

void log_warning(struct log *__restrict l, 
                 const char *__restrict tag,
                 const char *__restrict fmt,
                 ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    print(l, LOG_WARNING, tag, fmt, vargs);
    va_end(vargs);
}

void log_error(struct log *__restrict l, 
               const char *__restrict tag,
               const char *__restrict fmt,
               ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    print(l, LOG_ERROR, tag, fmt, vargs);
    va_end(vargs);
}

void log_clear(struct log *__restrict l)
{
    int fd, err;
    
    fd = fileno(l->file);
    
    do {
        err = ftruncate(fd, 0);
    } while(err < 0 && errno == EINTR);
}

void log_print(struct log *__restrict l, int fd)
{
    char *line;
    size_t size;
    ssize_t n; 
    
    line = NULL;
    size = 0;
    
    rewind(l->file);
    
    while(1) {
        n = getline(&line, &size, l->file);
        if(n < 0)
            break;
        
        dprintf(fd, "%s", line);
    }
    
    free(line);
}