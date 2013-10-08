#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <clock.h>

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