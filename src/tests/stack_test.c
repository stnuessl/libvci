#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <stack.h>

void push_string(struct stack *__restrict stack, const char *str)
{
    int err;
    
    while(*str != '\0') {
        /* surpress warnings ;-) */
        err = stack_push(stack, (void *)(unsigned long) *str++);
        assert(err == 0);
    }
}

bool is_palindrome(struct stack *__restrict stack, const char *str)
{
    char c;

    push_string(stack, str);
    
    c = EOF;
    
    while(!stack_empty(stack)) {
        c = (unsigned long) stack_pop(stack);
               
        if(c != *str++)
            break;
    }
    
    stack_clear(stack, NULL);

    return c == *--str;
}

int main(int argc, char *argv[])
{
    struct stack *stack;
    char *str;
    
    if(argc < 2) {
        fprintf(stdout, "*WARNING* :: missing argument(s).\n"
                        "Usage:\n\t./stack_test string1 string2 string3 ...\n");
        return EXIT_FAILURE;
    }
    
    /* skip name of the program */
    argv += 1;
    argc -= 1;
    
    stack = stack_new(strlen(argv[0]));
    assert(stack);
    
    while(argc--) {
        str = *argv++;
        
        if(is_palindrome(stack, str))
            fprintf(stdout, "%s is a palindrome!\n", str);
        else
            fprintf(stdout, "%s is not a palindrome!\n", str);
    }
    
    stack_delete(stack, NULL);
    
    return EXIT_SUCCESS;
}