#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "clock.h"

static inline void _timespec_reset(struct timespec *__restrict ts)
{ 
    ts->tv_sec  = 0;
    ts->tv_nsec = 0;
}

static inline struct timespec 
_timespec_diff(const struct timespec *__restrict t1,
               const struct timespec *__restrict t2)
{
    struct timespec ts;
    
    ts.tv_nsec = t1->tv_nsec - t2->tv_nsec;
    ts.tv_sec  = t1->tv_sec - t2->tv_sec;
    
    if(ts.tv_nsec < 0) {
        ts.tv_nsec += 1e9;
        ts.tv_sec  -= 1;
    }
    
    return ts;
}

static struct timespec _clock_elapsed(struct clock *__restrict c)
{
    if(c->_active)
        clock_gettime(c->_clk_id, &c->_current);
    
    return _timespec_diff(&c->_current, &c->_start);
}

struct clock *clock_new(clockid_t clk_id)
{
    struct clock *c;
    
    c = malloc(sizeof(*c));
    if(!c)
        return NULL;
    
    clock_init(c, clk_id);
    
    return c;
}

void clock_delete(struct clock *__restrict c)
{
    clock_destroy(c);
    free(c);
}

int clock_init(struct clock *__restrict c, clockid_t clk_id)
{
    int err;
    
    /* 
     * if these calls won't fail, the library assumes that
     * subsequent calls will also succeed
     */
    
    err = clock_gettime(clk_id, &c->_start);
    if(err < 0)
        return -errno;
    
    err = clock_gettime(clk_id, &c->_current);
    if(err < 0)
        return -errno;
    
    clock_clear(c);
    
    c->_clk_id = clk_id;
    
    return 0;
}

void clock_destroy(struct clock *__restrict c)
{
    clock_clear(c);
}

void clock_clear(struct clock *__restrict c)
{
    _timespec_reset(&c->_start);
    _timespec_reset(&c->_current);
    
    c->_active = false;
}

void clock_reset(struct clock *__restrict c)
{
    _timespec_reset(&c->_current);
    
    if(c->_active)
        clock_start(c);
    else
        _timespec_reset(&c->_start);
}

void clock_start(struct clock *__restrict c)
{
    clock_gettime(c->_clk_id, &c->_start);
    c->_active = true;
}

void clock_stop(struct clock *__restrict c)
{
    clock_gettime(c->_clk_id, &c->_current);
    c->_active = false;
}

void clock_continue(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    clock_gettime(c->_clk_id, &c->_start);
    
    c->_start  = _timespec_diff(&c->_start, &ts);
    c->_active = true;
}

unsigned long clock_elapsed_ms(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
}

unsigned long clock_elapsed_us(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * 1000000UL + ts.tv_nsec / 1000UL;
}

unsigned long clock_elapsed_ns(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * 1000000000UL + ts.tv_nsec % 1000000000UL;
}