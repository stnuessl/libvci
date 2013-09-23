#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <item.h>
#include <stack.h>


void push_string(struct stack *__restrict stack, const char *str)
{
    int err;
    char *c;
    
    while(*str != '\0') {
        c = malloc(sizeof(*c));
        assert(c);
        
        *c = *str++;
        
        err = stack_push(stack, c, NULL);
        assert(!(err < 0));
    }
}

bool is_palindrome(const char *str)
{
    struct stack stack;
    struct item *item;
    char c;
    
    stack_init(&stack);
    
    c = EOF;
    
    stack_set_data_delete(&stack, &free);
    push_string(&stack, str);
    
    while(!stack_empty(&stack)) {
        item = stack_pop_item(&stack);
        c = *(char *)item_data(item);
        
        item_delete(item, &free, NULL);
       
        if(c != *str++)
            break;
    }

    stack_destroy(&stack);

    return c == *--str;
}

int main(int argc, char *argv[])
{
    char *str;
    
    if(argc < 2) {
        fprintf(stdout, "*WARNING* :: missing argument(s).\n"
                        "Usage:\n\t./stack_test string1 string2 string3 ...\n");
        return EXIT_SUCCESS;
    }
    
    while(argc--) {
        str = *argv++;
        if(is_palindrome(str))
            fprintf(stdout, "%s is a palindrome!\n", str);
        else
            fprintf(stdout, "%s is not a palindrome!\n", str);
    }
    
    return EXIT_SUCCESS;
}