#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <link.h>
#include <stack.h>
#include <macro.h>

struct data {
    char data;
    struct link link;
};

char *words[] = {
    "hannah",
    "maniac",
    "deified",
    "haha",
    "i",
    "kayak"
};

bool is_palindrome(struct stack *__restrict stack, const char *str)
{
    struct data *data, *tmp;
    char c;
    int len, i;
    
    len = strlen(str);
    
    data = calloc(len, sizeof(*data));
    
    for(i = 0; i < len; ++i) {
        data[i].data = str[i];
        
        stack_insert(stack, &data[i].link);
    }
    
    c = EOF;

    while(!stack_empty(stack)) {
        tmp = container_of(stack_pop(stack), struct data, link);
        
        c = tmp->data;
        
        if(c != *str++)
            break;
    }
    
    stack_clear(stack, NULL);
    free(data);

    return c == *--str;
}

int main(int argc, char *argv[])
{
    struct stack *stack;
    char *str;
    
    if(argc < 2) {
        argv = words;
        argc = ARRAY_SIZE(words);
    }
    
    /* skip name of the program */
    argv += 1;
    argc -= 1;
    
    stack = stack_new();
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