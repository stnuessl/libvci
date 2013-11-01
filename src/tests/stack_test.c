#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <item.h>
#include <stack.h>

#include <macros.h>

void push_string(struct stack *__restrict stack, const char *str)
{
    int err;
    
    while(*str != '\0') {
        /* surpress warnings ;-) */
        err = stack_push(stack, (void *)(unsigned long) *str++, NULL);
        assert(err == 0);
    }
}

bool is_palindrome(const char *str)
{
    struct stack stack __on_return(stack_destroy);
    struct item *item;
    char c;
    
    stack_init(&stack);
    
    push_string(&stack, str);
    
    while(!stack_empty(&stack)) {
        item = stack_pop_item(&stack);
        c = (unsigned long) item_data(item);
        
        item_delete(item, NULL, NULL);
       
        if(c != *str++)
            return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    char *str;
    
    if(argc < 2) {
        fprintf(stdout, "*WARNING* :: missing argument(s).\n"
                        "Usage:\n\t./stack_test string1 string2 string3 ...\n");
        return EXIT_SUCCESS;
    }
    
    /* skip name of the program */
    argv += 1;
    argc -= 1;
    
    while(argc--) {
        str = *argv++;
        if(is_palindrome(str))
            fprintf(stdout, "%s is a palindrome!\n", str);
        else
            fprintf(stdout, "%s is not a palindrome!\n", str);
    }
    
    return EXIT_SUCCESS;
}