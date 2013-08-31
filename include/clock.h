#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <time.h>
#include <stdbool.h>

struct clock {
    struct timespec _start;
    struct timespec _current;
    
    clockid_t _clk_id;
    
    bool _active;
};

struct clock *clock_new(clockid_t clk_id);

void clock_delete(struct clock *__restrict c);

int clock_init(struct clock *__restrict c, clockid_t clk_id);

void clock_destroy(struct clock *__restrict c);

void clock_clear(struct clock *__restrict c);

void clock_reset(struct clock *__restrict c);

void clock_start(struct clock *__restrict c);

void clock_stop(struct clock *__restrict c);

void clock_continue(struct clock *__restrict c);

unsigned long clock_elapsed_ms(struct clock *__restrict c);

unsigned long clock_elapsed_us(struct clock *__restrict c);

unsigned long clock_elapsed_ns(struct clock *__restrict c);

#endif /* _CLOCK_H_ */