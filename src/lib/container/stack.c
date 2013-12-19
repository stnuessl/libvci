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


void stack_push(struct stack *__restrict stack, struct link *link)
{
    stack->size += 1;
    
    list_insert_front(&stack->list, link);
}

struct link *stack_pop(struct stack *__restrict stack)
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