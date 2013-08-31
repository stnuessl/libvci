
#ifndef _ETRACE_H_
#define _ETRACE_H_

#include <stdio.h>
#include <stdbool.h>


int etrace_init(int size, int fd);

void etrace_destroy(bool close_fd);

void etrace_backtrace(int skip);

void etrace_printf(const char *__restrict fmt, ...) 
                   __attribute__((format(printf,1,2)));

int etrace_backtrace_on_signals(bool trace, const int *signals, int size);

#if DEBUG >= 2

#define etrace_error(name, err)                                                \
    do {                                                                       \
        etrace_printf("ERROR :: %s() failed with %d in %s:%d\n",               \
                      (name), (err), __FILE__, __LINE__);                      \
                                                                               \
        etrace_backtrace(0);                                                   \
    } while(0)
    
#elif DEBUG == 1

#define etrace_error(name, err)                                                \
    etrace_printf("ERROR :: %s() failed with %d in %s:%d\n",                   \
                  (name), (err), __FILE__, __LINE__)
    
#else

#define etrace_error(name, err) 

#endif

#endif /* _ETRACE_H_ */