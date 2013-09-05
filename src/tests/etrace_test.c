#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <etrace.h>

unsigned long bad_fakulty(unsigned int n)
{
    if(n > 1)
        return n * bad_fakulty(n - 1);
    else
        return 1 / (n - 1);
}

void segfault(void)
{
    *(int *) 0 = 0;
}

void to(void)
{
    segfault();
}

void path(void)
{
    to();
}

int make_error(void)
{
   return -1;
}

void error(void)
{
    int err;
    
    err = make_error(); 
    if(err < 0)
        etrace_error("make_error", err);
}

int main(int argc, char *argv[])
{
    int err;
    char *s;
    
    if(etrace_init(16, NULL) < 0)
        abort();
    
    if(argc > 2) {
        etrace_write("Aborting: Too many arguments.\n");
        abort();
    }
    
    if(argc == 2)
        s = argv[1];
    else
        s = "";
    
    err = etrace_backtrace_on_signals(true, NULL, 0);
    if(err < 0) {
        etrace_error("trace_on_signals", -err);
        abort();
    }
    
    if(!strcmp(s, "--segfault"))
        path();
    else if(!strcmp(s, "--float"))
        printf("%lu\n", bad_fakulty(10));
    else if(!strcmp(s, "--abort"))
        abort();
    else if(!strcmp(s, "--error"))
        error();
    else
        fprintf(stdout, "Finished successful\n");
    
    etrace_destroy();
    
    return EXIT_SUCCESS;
}