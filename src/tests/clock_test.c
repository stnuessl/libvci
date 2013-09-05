#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <clock.h>

int main(int argc, char *argv[])
{
    struct clock c;
    int err;
    
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
    
    clock_destroy(&c);
    
    return EXIT_SUCCESS;
}