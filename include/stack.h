
#ifndef _STACK_H_
#define _STACK_H_

#include <stdbool.h>

#include "link.h"

struct stack {
    struct link list;
    
    unsigned int size;
};


struct stack *stack_new(void);

void stack_delete(struct stack *__restrict stack, 
                  void (*data_delete)(struct link *));

void stack_init(struct stack *__restrict stack);

void stack_destroy(struct stack *__restrict stack, 
                   void (*data_delete)(struct link *));

void stack_clear(struct stack *__restrict stack, 
                 void (*data_delete)(struct link *));

void stack_insert(struct stack *__restrict stack, struct link *link);

struct link *stack_take(struct stack *__restrict stack);

struct link *stack_top(struct stack *__restrict stack);

int stack_size(const struct stack *__restrict stack);

bool stack_empty(const struct stack *__restrict stack);

#endif /* _STACK_H_ */