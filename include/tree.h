#ifndef _TREE_H_
#define _TREE_H_

#include <stdbool.h>

#include "item.h"

struct tree {
    struct item *_root;
    
    int _size;
    
    int (*_key_compare)(const void *, const void *);
    void (*_data_delete)(void *);
    void (*_key_delete)(void *);
    void (*_callback)(struct item *, void *);
};


struct tree *tree_new(void);

void tree_delete(struct tree *__restrict tree);

void tree_init(struct tree *__restrict tree);

void tree_destroy(struct tree *__restrict tree);

void tree_pre_order(struct tree *__restrict tree, void *__restrict arg);

void tree_in_order(struct tree *__restrict tree, void *__restrict arg);

void tree_post_order(struct tree *__restrict tree, void *__restrict arg);

inline int tree_size(const struct tree *__restrict tree);

inline bool tree_empty(const struct tree *__restrict tree);


#define TREE_DEFINE_SET_CALLBACK(name, type, param)                            \
                                                                               \
void tree_set_##name(struct tree *__restrict tree, type (*name)param);
    
TREE_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
TREE_DEFINE_SET_CALLBACK(data_delete, void, (void *))
TREE_DEFINE_SET_CALLBACK(key_delete, void, (void *))
TREE_DEFINE_SET_CALLBACK(callback, void, (struct item *, void *))
    
#undef TREE_DEFINE_SET_CALLBACK

#endif /* _TREE_H_ */