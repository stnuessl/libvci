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
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <libvci/link.h>
#include <libvci/stack.h>
#include <libvci/macro.h>

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
        tmp = container_of(stack_take(stack), struct data, link);
        
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