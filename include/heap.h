#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdbool.h>

#include "item.h"
#include "list.h"
#include "tree.h"

int heap_insert(struct tree *__restrict tree, 
                void *__restrict data, 
                void *__restrict key);

void heap_insert_item(struct tree *__restrict tree, 
                      struct item *__restrict item);

void *heap_take_root(struct tree *__restrict tree);

struct item *heap_take_root_item(struct tree *__restrict tree);

void *heap_root(struct tree *__restrict tree);

struct item *heap_root_item(struct tree *__restrict tree);

struct list heap_sort(struct tree *__restrict tree);


#endif /* _HEAP_H_ */