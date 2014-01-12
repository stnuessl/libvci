#ifndef _LOG_H_
#define _LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "clock.h"

#define LOG_PRINT_DATE          (1 << 0)
#define LOG_PRINT_TIMESTAMP     (1 << 1)
#define LOG_PRINT_PID           (1 << 2)
#define LOG_PRINT_NAME          (1 << 3)
#define LOG_PRINT_HOSTNAME      (1 << 4)
#define LOG_PRINT_LEVEL         (1 << 5)

#define LOG_SEVERITY_INFO     5
#define LOG_SEVERITY_DEBUG    4
#define LOG_SEVERITY_MESSAGE  3
#define LOG_SEVERITY_WARNING  2
#define LOG_SEVERITY_CRITICAL 1
#define LOG_SEVERITY_ERROR    0

struct log {
    char *name;
    char *hostname;
    FILE *file;
    struct clock clock;
    
    uint8_t flags;
    uint8_t severity_cap;
};

struct log *log_new(const char *__restrict path, 
                    const char *__restrict name, 
                    uint8_t flags);

void log_delete(struct log *__restrict l);

int log_init(struct log *__restrict l,
             const char *__restrict path,
             const char *__restrict name, 
             uint8_t flags);

void log_destroy(struct log *__restrict l);

void log_set_file(struct log *__restrict l, FILE *f);

inline int log_fd(const struct log *__restrict l);

inline void log_set_severity_cap(struct log *__restrict l, int severity_cap);

inline int log_severity_cap(const struct log *__restrict l);

void log_printf(struct log *__restrict l, 
               int level, 
               const char *__restrict fmt, ...)
                                            __attribute__((format(printf,3,4)));

void log_vprintf(struct log *__restrict l,
                  int level,
                  const char *__restrict fmt, va_list vargs);

void log_info(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_debug(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_message(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_warning(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_critical(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

void log_error(struct log *__restrict l, const char *__restrict fmt, ...)
                                            __attribute__((format(printf,2,3)));

#endif /* _LOG_H_ */