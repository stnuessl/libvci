#ifndef _ITEM_ALLOCATOR_H_
#define _ITEM_ALLOCATOR_H_

#include "mempool.h"
#include "item.h"

int item_allocator_init(int size);

void item_allocator_destroy(void);

struct item *item_allocator_alloc(void);

inline void item_allocator_free(struct item *__restrict item);

#endif /* _ITEM_ALLOCATOR_H_ */