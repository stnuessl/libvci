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

static const char *_log_severity_string(int severity)
{
    static const char *strings[] = {
        "ERROR",
        "CRITICAL",
        "WARNING",
        "MESSAGE",
        "DEBUG",
        "INFO"
    };
    
    return strings[severity];
}

static void _log_write_line_header(struct log *__restrict l, int level)
{
    time_t now;
    struct tm ltime;
    
    if(l->_flags & LOG_PRINT_DATE) {
        now = time(NULL);
        
        localtime_r(&now, &ltime);
        
        fprintf(l->_f, "%04d-%02d-%02d :: %02d:%02d:%02d ",
                ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday,
                ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
    }
    
    if(l->_flags & LOG_PRINT_HOSTNAME)
        fprintf(l->_f, "| %s | ", l->_hostname);
        
    if(l->_flags & LOG_PRINT_TIMESTAMP) {
        fprintf(l->_f, "[ %*lf ] ",
                10, (double) clock_elapsed_us(&l->_clock) / 1e6);
    }
    
    if(l->_flags & LOG_PRINT_NAME)
        fprintf(l->_f,  "< %s > ", l->_name);
    
    if(l->_flags & LOG_PRINT_PID)
        fprintf(l->_f, "( %*u ) ", 5, getpid());
        
    if(l->_flags & LOG_PRINT_LEVEL)
        fprintf(l->_f, "{ %*s } ", 9, _log_severity_string(level));
    
    fprintf(l->_f, ": ");
}


struct log *log_new(const char *__restrict path, 
                    const char *__restrict name, 
                    uint8_t flags)
{
    struct log *l;
    int err;
    
    l = malloc(sizeof(*l));
    if(!l)
        return NULL;
    
    err = log_init(l, path, name, flags);
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
             const char *__restrict name, 
             uint8_t flags)
{
#define HOSTNAME_SIZE 64
    char hostname[HOSTNAME_SIZE];
    int err;
    
    l->_name = strdup(name);
    if(!l->_name) {
        err = -errno;
        goto out;
    }
    
    l->_f = fopen(path, "a");
    if(!l->_f) {
        err = -errno;
        goto cleanup1;
    }
    
    if(flags & LOG_PRINT_TIMESTAMP) {
        err = clock_init(&l->_clock, CLOCK_MONOTONIC);
        if(err < 0)
            goto cleanup2;
        
        clock_start(&l->_clock);
    }
    
    if(flags & LOG_PRINT_HOSTNAME) {
        hostname[HOSTNAME_SIZE - 1] = '\0';
        
        err = gethostname(hostname, HOSTNAME_SIZE - 1);
        if(err < 0)
            goto cleanup3;
        
        l->_hostname = strdup(hostname);
        if(!l->_hostname) {
            err = -errno;
            goto cleanup3;
        }
    }

    l->_flags = flags;
    l->_severity_cap = LOG_SEVERITY_INFO;
    
    return 0;

cleanup3:
    if(flags & LOG_PRINT_TIMESTAMP)
        clock_destroy(&l->_clock);
cleanup2:
    fclose(l->_f);
cleanup1:
    free(l->_name);
out:
    return err;
}

void log_destroy(struct log *__restrict l)
{
    if(l->_flags & LOG_PRINT_TIMESTAMP)
        clock_destroy(&l->_clock);
    
    if(l->_flags & LOG_PRINT_HOSTNAME)
        free(l->_hostname);
    
    free(l->_name);
    
    fclose(l->_f);
}

void log_set_file(struct log *__restrict l, FILE *f)
{
    if(l->_f)
        fclose(l->_f);

    l->_f  =  f;
}

inline int log_fd(const struct log *__restrict l)
{
    return fileno(l->_f);
}

inline void log_set_severity_cap(struct log *__restrict l, int severity_cap)
{
    l->_severity_cap = severity_cap;
}

inline int log_severity_cap(const struct log *__restrict l)
{
    return l->_severity_cap;
}

static void _log_printf(struct log *__restrict l,
                       int severity,
                       const char *__restrict fmt,
                       va_list vargs)
{
    if(severity > l->_severity_cap)
        return;

    _log_write_line_header(l, severity);
    vfprintf(l->_f, fmt, vargs);
    fflush(l->_f);
}

void log_printf(struct log *__restrict l, 
               int severity, 
               const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, severity, fmt, vargs);
    va_end(vargs);
}

void log_vprintf(struct log *__restrict l,
                  int level,
                  const char *__restrict fmt, va_list vargs)
{
    _log_printf(l, level, fmt, vargs);
}

void log_info(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_INFO, fmt, vargs);
    va_end(vargs);
}

void log_debug(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_DEBUG, fmt, vargs);
    va_end(vargs);
}

void log_message(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_MESSAGE, fmt, vargs);
    va_end(vargs);
}

void log_warning(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_WARNING, fmt, vargs);
    va_end(vargs);
}

void log_critical(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_CRITICAL, fmt, vargs);
    va_end(vargs);
}

void log_error(struct log *__restrict l, const char *__restrict fmt, ...)
{
    va_list vargs;
    
    va_start(vargs, fmt);
    _log_printf(l, LOG_SEVERITY_ERROR, fmt, vargs);
    va_end(vargs);
}