
#ifndef _ETRACE_H_
#define _ETRACE_H_

int etrace_init(const char *__restrict path);

void etrace_destroy(void);

void etrace_set_file(FILE *__restrict f);

void etrace_backtrace(int skip);

void etrace_write(const char *__restrict fmt, ...) 
                   __attribute__((format(printf,1,2)));

int etrace_backtrace_on_signals(bool trace, const int *signals, int size);

#if ETRACE_DEBUG >= 2

#define etrace_error(name, err)                                                \
    do {                                                                       \
        etrace_write("%s() failed with %d in %s:%d\n",                         \
                      (name), (err), __FILE__, __LINE__);                      \
                                                                               \
        etrace_backtrace(0);                                                   \
    } while(0)
    
#elif ETRACE_DEBUG == 1

#define etrace_error(name, err)                                                \
    etrace_write("%s() failed with %d in %s:%d\n",                             \
                  (name), (err), __FILE__, __LINE__)
    
#else

#define etrace_error(name, err) 

#endif

#endif /* _ETRACE_H_ */