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
    if(c->active)
        clock_gettime(c->clk_id, &c->current);
    
    return _timespec_diff(&c->current, &c->start);
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
    
    err = clock_gettime(clk_id, &c->start);
    if(err < 0)
        return -errno;
    
    err = clock_gettime(clk_id, &c->current);
    if(err < 0)
        return -errno;
    
    clock_clear(c);
    
    c->clk_id = clk_id;
    
    return 0;
}

void clock_destroy(struct clock *__restrict c)
{
    clock_clear(c);
}

void clock_clear(struct clock *__restrict c)
{
    _timespec_reset(&c->start);
    _timespec_reset(&c->current);
    
    c->active = false;
}

void clock_reset(struct clock *__restrict c)
{
    _timespec_reset(&c->current);
    
    if(c->active)
        clock_start(c);
    else
        _timespec_reset(&c->start);
}

void clock_start(struct clock *__restrict c)
{
    clock_gettime(c->clk_id, &c->start);
    c->active = true;
}

void clock_stop(struct clock *__restrict c)
{
    clock_gettime(c->clk_id, &c->current);
    c->active = false;
}

void clock_continue(struct clock *__restrict c)
{
    struct timespec ts;
    
    /* get already elapsed time */
    ts = _clock_elapsed(c);
    
    /* get new start time */
    clock_gettime(c->clk_id, &c->start);
    
    /* push the start time further into the past by the already elapsed time */
    c->start  = _timespec_diff(&c->start, &ts);
    c->active = true;
}

unsigned long clock_elapsed_ms(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * 1000UL + ts.tv_nsec / (unsigned long) 1e6;
}

unsigned long clock_elapsed_us(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * (unsigned long) 1e6 + ts.tv_nsec / 1000UL;
}

unsigned long clock_elapsed_ns(struct clock *__restrict c)
{
    struct timespec ts;
    
    ts = _clock_elapsed(c);
    
    return ts.tv_sec * (unsigned long) 1e9 + ts.tv_nsec % (unsigned long) 1e9 ;
}