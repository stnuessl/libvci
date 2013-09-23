
#include <stdlib.h>
#include <errno.h>

#include "item.h"
#include "list.h"
#include "stack.h"


#define STACK_DEFINE_STATIC_SETGET(name, type)                                 \
                                                                               \
static inline void _stack_set_##name(struct stack *__restrict stack, type name)\
{                                                                              \
    stack->_##name = name;                                                     \
}                                                                              \
                                                                               \
static inline type _stack_get_##name(struct stack *__restrict stack)           \
{                                                                              \
    return stack->_##name;                                                     \
}                                                                              \

#undef STACK_DEFINE_STATIC_SETGET

struct stack *stack_new(void)
{
    struct stack *stack;
    
    stack = malloc(sizeof(*stack));
    if(!stack)
        return NULL;

    stack_init(stack);
    
    return stack;
}

void stack_delete(struct stack *__restrict stack)
{
    stack_destroy(stack);
    free(stack);
}

void stack_init(struct stack *__restrict stack)
{
    list_init(&stack->_list);
}

void stack_destroy(struct stack *__restrict stack)
{
    list_destroy(&stack->_list, stack->_data_delete, stack->_key_delete);
}

int stack_push(struct stack *__restrict stack, 
                    void *__restrict data, 
                    void *__restrict key)
{
    struct item *item;
    
    item = item_new(data, key);
    if(!item)
        return -errno;
    
    stack_push_item(stack, item);
    
    return 0;
}

void stack_push_item(struct stack *__restrict stack, 
                     struct item *__restrict item)
{
    list_insert_item_front(&stack->_list, item);
}

void *stack_pop(struct stack *__restrict stack)
{
    struct item *item;
    void *data;
    
    item = stack_pop_item(stack);
    
    data = item_data(item);
    
    item_delete(item, NULL, stack->_key_delete);
    
    return data;
}

struct item *stack_pop_item(struct stack *__restrict stack)
{
    return list_take_item_front(&stack->_list);
}

void *stack_top(struct stack *__restrict stack)
{
    return item_data(stack_top_item(stack));
}

struct item *stack_top_item(struct stack *__restrict stack)
{
    return list_begin(&stack->_list);
}

void stack_delete_top(struct stack *__restrict stack)
{
    item_delete(stack_pop_item(stack), stack->_data_delete, stack->_key_delete);
}

int stack_size(const struct stack *__restrict stack)
{
    return list_size(&stack->_list);
}

bool stack_empty(const struct stack *__restrict stack)
{
    return list_empty(&stack->_list);
}

#define STACK_DEFINE_SET_CALLBACK(name, type, param)                           \
                                                                               \
inline void stack_set_##name(struct stack *__restrict stack,                   \
                             type (*name)param)                                \
{                                                                              \
    stack->_##name = name;                                                     \
}

STACK_DEFINE_SET_CALLBACK(key_delete, void, (void *))
STACK_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef STACK_DEFINE_SET_CALLBACK