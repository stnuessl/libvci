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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <time.h>
#include <stdbool.h>

struct clock {
    struct timespec start;
    struct timespec current;
    
    clockid_t clk_id;
    
    bool active;
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