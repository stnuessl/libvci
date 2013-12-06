#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "container_util.h"
#include "stack.h"

#define STACK_DEFAULT_CAPACITY 32

struct stack *stack_new(unsigned int capacity)
{
    struct stack *stack;
    int err;
    
    stack = malloc(sizeof(*stack));
    if(!stack)
        return NULL;
    
    err = stack_init(stack, capacity);
    if(err < 0) {
        free(stack);
        return NULL;
    }
    
    return stack;
}

void stack_delete(struct stack *__restrict stack, void (*data_delete)(void *))
{
    stack_destroy(stack, data_delete);
    free(stack);
}

int stack_init(struct stack *__restrict stack, unsigned int capacity)
{
    capacity = adjust(capacity, STACK_DEFAULT_CAPACITY);
    
    stack->data = calloc(capacity, sizeof(*stack->data));
    if(!stack->data)
        return -errno;
    
    stack->size = 0;
    stack->capacity = capacity;
    
    return 0;
}

void stack_destroy(struct stack *__restrict stack, void (*data_delete)(void *))
{
    stack_clear(stack, data_delete);
    free(stack->data);
}

void stack_clear(struct stack *__restrict stack, void (*data_delete)(void *))
{
    if(!data_delete) {
        stack->size = 0;
        return;
    }
    
    while(stack->size--)
        data_delete(stack->data[stack->size]);
}


int stack_push(struct stack *__restrict stack, void *data)
{
    int err;
    
    if(stack->size >= stack->capacity) {
        err = stack_set_capacity(stack, stack->capacity << 1);
        if(err < 0)
            return err;
    }
    
    stack->data[stack->size] = data;
    stack->size += 1;
    
    return 0;
}
// 
void *stack_pop(struct stack *__restrict stack)
{
    stack->size -= 1;
    
    return stack->data[stack->size];
}

inline void *stack_top(struct stack *__restrict stack)
{
    return stack->data[stack->size - 1];
}

inline int stack_size(const struct stack *__restrict stack)
{
    return stack->size;
}

inline bool stack_empty(const struct stack *__restrict stack)
{
    return stack->size == 0;
}

int stack_set_capacity(struct stack *__restrict stack, unsigned int capacity)
{
    void **data;
    
    capacity = adjust(capacity, STACK_DEFAULT_CAPACITY);
    
    if(capacity == stack->capacity)
        return 0;
    
    data = realloc(stack->data, capacity * sizeof(*stack->data));
    if(!data)
        return -errno;
    
    if(stack->size > capacity)
        stack->size = capacity;
    
    stack->data = data;
    stack->capacity = capacity;
    
    return 0;
}

int stack_squeeze(struct stack *__restrict stack)
{
    return stack_set_capacity(stack, stack->size);
}