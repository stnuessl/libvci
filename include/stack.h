
#ifndef _STACK_H_
#define _STACK_H_

#include "item.h"
#include "list.h"

struct stack {
    struct list _list;
    
    void (*_data_delete)(void *);
    void (*_key_delete)(void *);
};


struct stack *stack_new(void);

void stack_delete(struct stack *__restrict stack);

void stack_init(struct stack *__restrict stack);

void stack_destroy(struct stack *__restrict stack);

int stack_push(struct stack *__restrict stack, 
               void *__restrict data, 
               void *__restrict key);

void stack_push_item(struct stack *__restrict stack, 
                     struct item *__restrict item);

void *stack_pop(struct stack *__restrict stack);

struct item *stack_pop_item(struct stack *__restrict stack);

void *stack_top(struct stack *__restrict stack);

struct item *stack_top_item(struct stack *__restrict stack);

int stack_size(const struct stack *__restrict stack);

bool stack_is_empty(const struct stack *__restrict stack);


#define STACK_DEFINE_SET_CALLBACK(name, type, param)                           \
                                                                               \
inline void stack_set_##name(struct stack *__restrict stack,                   \
                             type (*name)param);

STACK_DEFINE_SET_CALLBACK(key_delete, void, (void *))
STACK_DEFINE_SET_CALLBACK(data_delete, void, (void *))

#undef STACK_DEFINE_SET_CALLBACK


#endif /* _STACK_H_ */