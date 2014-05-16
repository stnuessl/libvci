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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "link.h"
#include "list.h"
#include "stack.h"


struct stack *stack_new(void)
{
    struct stack *stack;
    
    stack = malloc(sizeof(*stack));
    if(!stack)
        return NULL;
    
    stack_init(stack);
    
    return stack;
}

void stack_delete(struct stack *__restrict stack,
                  void (*data_delete)(struct link *))
{
    stack_destroy(stack, data_delete);
    free(stack);
}

void stack_init(struct stack *__restrict stack)
{
    list_init(&stack->list);
    
    stack->size = 0;
}

void stack_destroy(struct stack *__restrict stack, 
                   void (*data_delete)(struct link *))
{
    list_destroy(&stack->list, data_delete);
}

void stack_clear(struct stack *__restrict stack,
                 void (*data_delete)(struct link *))
{
    stack->size = 0;

    list_clear(&stack->list, data_delete);
}


void stack_insert(struct stack *__restrict stack, struct link *link)
{
    stack->size += 1;
    
    list_insert_front(&stack->list, link);
}

struct link *stack_take(struct stack *__restrict stack)
{
    stack->size -= 1;
    
    return list_take_front(&stack->list);
}

struct link *stack_top(struct stack *__restrict stack)
{
    return list_front(&stack->list);
}

int stack_size(const struct stack *__restrict stack)
{
    return stack->size;
}

bool stack_empty(const struct stack *__restrict stack)
{
    return stack->size == 0;
}