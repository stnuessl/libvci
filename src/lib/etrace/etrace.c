#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <execinfo.h>
#include <stdbool.h>
#include <stdarg.h>

#include "log.h"
#include "macro.h"
#include "etrace.h"

#define BACKTRACE_BUFFER_SIZE 16

static void *_buffer[BACKTRACE_BUFFER_SIZE];
static struct log *_log;
static int _fd;

static void _signal_handler(int signal)
{
    const char *msg = "ERROR :: Received signal - ";
    char *signal_name;
    
    signal_name = strsignal(signal);
    
    write(_fd, msg, strlen(msg));
    write(_fd, signal_name, strlen(signal_name));
    write(_fd, "\n", sizeof(char));
    
    /* skip this function and the glibc wrapper */
    etrace_backtrace(2);
    
    _exit(EXIT_FAILURE);
}

int etrace_init(const char *__restrict path)
{
    int flags;
    
    flags = LOG_PRINT_DATE | LOG_PRINT_TIMESTAMP | LOG_PRINT_PID | 
            LOG_PRINT_NAME | LOG_PRINT_LEVEL;
    
    _log = log_new(path, "etrace", flags);
    if(!_log)
        return -errno;
    
    _fd = log_fd(_log);

    return 0;
}

void etrace_destroy(void)
{
    log_delete(_log);
}

void etrace_set_file(FILE *__restrict f)
{
    log_set_file(_log, f);
    
    _fd = fileno(f);
}

void etrace_backtrace(int skip)
{
    const char *msg = "backtrace():\n";
    int size;
    
    size = backtrace(_buffer, BACKTRACE_BUFFER_SIZE);
    
    /* just print the backtrace down to main() */
    if(size > 3)
        size -= 2;

    /* skip this function on printed backtrace */
    if(skip + 1 < size)
        skip += 1;
    else
        skip = 0;
    
    write(_fd, msg, strlen(msg));
    
    backtrace_symbols_fd(_buffer + skip, size - skip, _fd);
    
    write(_fd, "\n", 1);
}

void etrace_write(const char *__restrict fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    log_vprintf(_log, LOG_SEVERITY_ERROR, fmt, args);
    va_end(args);
}

int etrace_backtrace_on_signals(bool trace, const int *signals, int size)
{
    struct sigaction sa;
    const int def_signals[] = { SIGABRT, SIGBUS, SIGFPE, SIGPIPE, SIGSEGV };
    int err;
    
    if(!signals) {
        signals = def_signals;
        size    = ARRAY_SIZE(def_signals);;
    }

    err = 0;
    
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = (trace) ? &_signal_handler : SIG_DFL;
    
    while(size--) {
        if(sigaction(signals[size], &sa, NULL) < 0) {
            etrace_error("sigaction", errno);
            err = -errno;
        }
    }
    
    return err;
}
