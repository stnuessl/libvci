
#ifndef _STACK_H_
#define _STACK_H_

#include <stdbool.h>

struct stack {
    void **data;
    unsigned int size;
    unsigned int capacity;
};


struct stack *stack_new(unsigned int capacity);

void stack_delete(struct stack *__restrict stack, void (*data_delete)(void *));

int stack_init(struct stack *__restrict stack, unsigned int capacity);

void stack_destroy(struct stack *__restrict stack, void (*data_delete)(void *));

void stack_clear(struct stack *__restrict stack, void (*data_delete)(void *));

int stack_push(struct stack *__restrict stack, void *data);

void *stack_pop(struct stack *__restrict stack);

inline void *stack_top(struct stack *__restrict stack);

inline int stack_size(const struct stack *__restrict stack);

inline bool stack_empty(const struct stack *__restrict stack);

int stack_set_capacity(struct stack *__restrict stack, unsigned int capacity);

int stack_squeeze(struct stack *__restrict stack);

#endif /* _STACK_H_ */