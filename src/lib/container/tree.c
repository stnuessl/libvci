#include <stdlib.h>
#include <string.h>

#include "item.h"
#include "tree.h"

static void _tree_destroy(struct tree *__restrict tree, 
                          struct item *__restrict root)
{
    if(!root)
        return;
    
    _tree_destroy(tree, item_left(root));
    _tree_destroy(tree, item_right(root));
    
    item_delete(root, tree->_data_delete, tree->_key_delete);
}

static void _tree_pre_order(struct tree *__restrict tree, 
                           struct item *__restrict root, 
                           void *__restrict arg)
{
    if(!root)
        return;
    
    tree->_callback(root, arg);
    
    _tree_pre_order(tree, item_left(root), arg);
    _tree_pre_order(tree, item_right(root), arg);
}

static void _tree_in_order(struct tree *__restrict tree, 
                            struct item *__restrict root, 
                            void *__restrict arg)
{
    if(!root)
        return;
    
    _tree_in_order(tree, item_left(root), arg);
    
    tree->_callback(root, arg);
    
    _tree_in_order(tree, item_right(root), arg);
    
}


static void _tree_post_order(struct tree *__restrict tree, 
                           struct item *__restrict root, 
                           void *__restrict arg)
{
    if(!root)
        return;
    
    _tree_post_order(tree, item_left(root), arg);
    _tree_post_order(tree, item_right(root), arg);
    
    tree->_callback(root, arg);
}

struct tree *tree_new(void)
{
    struct tree *tree;
    
    tree = malloc(sizeof(*tree));
    if(!tree)
        return NULL;
    
    tree_init(tree);
    
    return tree;
}

void tree_delete(struct tree *__restrict tree)
{
    tree_destroy(tree);
    free(tree);
}

void tree_init(struct tree *__restrict tree)
{
    memset(tree, 0, sizeof(*tree));
}

void tree_destroy(struct tree *__restrict tree)
{
    _tree_destroy(tree, tree->_root);
}

void tree_pre_order(struct tree *__restrict tree, void *__restrict arg)
{
    _tree_pre_order(tree, tree->_root, arg);
}

void tree_in_order(struct tree *__restrict tree, void *__restrict arg)
{
    _tree_in_order(tree, tree->_root, arg);
}

void tree_post_order(struct tree *__restrict tree, void *__restrict arg)
{
    _tree_post_order(tree, tree->_root, arg);
}

inline int tree_size(const struct tree *__restrict tree)
{
    return tree->_size;
}

inline bool tree_empty(const struct tree *__restrict tree)
{
    return tree_size(tree) == 0;
}


#define TREE_DEFINE_SET_CALLBACK(name, type, param)                            \
                                                                               \
void tree_set_##name(struct tree *__restrict tree, type (*name)param)          \
{                                                                              \
    tree->_##name = name;                                                      \
}

TREE_DEFINE_SET_CALLBACK(key_compare, int, (const void *, const void *))
TREE_DEFINE_SET_CALLBACK(data_delete, void, (void *))
TREE_DEFINE_SET_CALLBACK(key_delete, void, (void *))
TREE_DEFINE_SET_CALLBACK(callback, void, (struct item *, void *))

#undef TREE_DEFINE_SET_CALLBACK
