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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libvci/clock.h>
#include <libvci/macro.h>

void do_task(struct clock *__restrict c)
{
    printf("do_task()\nTime: %lu us\n", clock_elapsed_us(c));
}

int main(int argc, char *argv[])
{
    struct clock c;
    int loops, err;
    
    err = clock_init(&c, CLOCK_MONOTONIC);
    if(err < 0) {
        fprintf(stderr, "%s\n", strerror(-err));
        exit(EXIT_SUCCESS);
    }
    
    clock_start(&c);
    
    usleep(1000);
    
    clock_stop(&c);
    
    printf("time elapsed ms: %lu\n", clock_elapsed_ms(&c));
    printf("time elapsed us: %lu\n", clock_elapsed_us(&c));
    printf("time elapsed ns: %lu\n", clock_elapsed_ns(&c));
    
    usleep(500 * 1000);
    clock_reset(&c);
    clock_start(&c);
    
    usleep(3000);
    
    clock_reset(&c);
    
    usleep(800);
    
    clock_stop(&c);
    
    printf("time elapsed ms: %lu\n", clock_elapsed_ms(&c));
    printf("time elapsed us: %lu\n", clock_elapsed_us(&c));
    printf("time elapsed ns: %lu\n", clock_elapsed_ns(&c));
    
    clock_clear(&c);
    clock_start(&c);
    
    loops = 0;
    
    /* call a function every 500ms */
    
    while(loops < 10) {
        if(clock_elapsed_ms(&c) >= 500) {
            do_task(&c);
            clock_reset(&c);
            loops += 1;
        }
        
        /* time for other tasks */
        usleep(100000);
    }
    
    clock_destroy(&c);
    
    return EXIT_SUCCESS;
}