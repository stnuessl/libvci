#ifndef _LOG_H_
#define _LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "clock.h"

#define LOG_PRINT_DATE          (1 << 0)
#define LOG_PRINT_TIMESTAMP     (1 << 1)
#define LOG_PRINT_PID           (1 << 2)
#define LOG_PRINT_NAME          (1 << 3)
#define LOG_PRINT_HOSTNAME      (1 << 4)
#define LOG_PRINT_LEVEL         (1 << 5)

#define LOG_INFO     5
#define LOG_DEBUG    4
#define LOG_MESSAGE  3
#define LOG_WARNING  2
#define LOG_CRITICAL 1
#define LOG_ERROR    0

struct log {
    char *_name;
    char *_hostname;
    FILE *_f;
    struct clock _clock;
    
    uint8_t _flags;
    uint8_t _log_level;
};

struct log *log_new(const char *__restrict path, 
                    const char *__restrict name, 
                    uint8_t flags, 
                    uint8_t log_level);

void log_delete(struct log *__restrict l);

int log_init(struct log *__restrict l,
             const char *__restrict path,
             const char *__restrict name, 
             uint8_t flags, 
             uint8_t log_level);

void log_destroy(struct log *__restrict l);

int log_fd(struct log *__restrict l);

void log_write(struct log *__restrict l, 
               int level, 
               const char *__restrict fmt, ...);

void log_info(struct log *__restrict l, const char *__restrict fmt, ...);

void log_debug(struct log *__restrict l, const char *__restrict fmt, ...);

void log_message(struct log *__restrict l, const char *__restrict fmt, ...);

void log_warning(struct log *__restrict l, const char *__restrict fmt, ...);

void log_critical(struct log *__restrict l, const char *__restrict fmt, ...);

void log_error(struct log *__restrict l, const char *__restrict fmt, ...);

#endif /* _LOG_H_ */