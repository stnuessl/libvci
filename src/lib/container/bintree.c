#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "item.h"
#include "bintree.h"


static struct item *_bintree_get(struct tree *__restrict tree,
                   struct item **root, 
                   struct item **(*item_get_reference)(struct item *__restrict), 
                   bool take);

inline static void _bintree_adjust_size(struct tree *__restrict tree,
                                        int adj)
{
    tree->_size += adj;
}

static struct item* _bintree_retrieve_item(struct tree *__restrict tree,
                                    struct item *__restrict root,
                                    void *__restrict key)
{
    int res;
    
    if(!root)
        return NULL;
    
    res = tree->_key_compare(key, item_key(root));
    
    if(res < 0)
        return _bintree_retrieve_item(tree, item_prev(root), key);
    else if(res > 0)
        return _bintree_retrieve_item(tree, item_next(root), key);
    else
        return root;
}

static bool _bintree_insert(struct tree *__restrict tree,
                            struct item **root,
                            struct item *__restrict item)
{
    int res;
    
    if(!*root) {
        *root = item;
        return true;
    }
    
    res = tree->_key_compare(item_key(item), item_key(*root));
    
    if(res < 0)
        return _bintree_insert(tree, item_ref_left(*root), item);
    else if(res > 0)
        return _bintree_insert(tree, item_ref_right(*root), item);
    else
        return false;
}


static struct item *_bintree_take(struct tree *__restrict tree,
                                  struct item **root,
                                  void *__restrict key)
{
    struct item *item, *left, *right, *min, **ref;
    int res;
    
    if(!*root)
        return NULL;
    
    res = tree->_key_compare(key, item_key(*root));
    
    if(res < 0) {
        return _bintree_take(tree, item_ref_left(*root), key);
    } else if(res > 0) {
        return _bintree_take(tree, item_ref_right(*root), key);
    } else {

        item = *root;
        
        left  = item_prev(*root);
        right = item_next(*root);
        
        
        if(!left || !right) {
            /* case: zero or one children */
            if(left)
                *root = left;
            else
                *root = right;
        } else {
            /* case: two children */
            ref = item_ref_right(*root);
            min = _bintree_get(tree, ref, &item_ref_left, true);
            
            item_swap(*root, min);
            
            return min;
        }
        
        _bintree_adjust_size(tree, -1);
        return item;
    }
}

static struct item *_bintree_get(struct tree *__restrict tree,
                   struct item **root, 
                   struct item **(*item_get_reference)(struct item *__restrict), 
                   bool take)
{
    struct item **item;
    
    item = item_get_reference(*root);
    
    if(!*item) {
        if(take)
            return _bintree_take(tree, root, item_key(*root));
 
        return *root;
    } else {
        return _bintree_get(tree, item, item_get_reference, take);
    }
}

static struct item *_bintree_leaf_item(struct tree *__restrict tree,
                                       struct item **root,
                                       struct item **(*to_leaf)(struct item *),
                                       bool remove)
{
    struct item **next;
    
    next = to_leaf(*root);
    if(next)
        return _bintree_leaf_item(tree, next, to_leaf, remove);
    
    if(remove)
        return _bintree_take(tree, root, item_key(*root));
    else
        return *root;
}

static void *_bintree_take_leaf(struct tree *__restrict tree,
                                struct item *(*take_item)(struct tree *))
{
    struct item *item;
    void *data;
    
    item = take_item(tree);
    
    data = item_data(item);
    
    item_delete(item, NULL, tree->_key_delete);
    
    return data;
}

bool bintree_insert(struct tree *__restrict tree, 
                    void *__restrict data, 
                    void *__restrict key)
{
    struct item *item;
    bool ok;
    
    item = item_new(data, key);
    if(!item)
        return false;
    
    ok = bintree_insert_item(tree, item);
    if(!ok)
        item_delete(item, NULL, NULL);
    
    return ok;
}

bool bintree_insert_item(struct tree *__restrict tree, 
                         struct item *__restrict item)
{
    bool ret;
    
    ret = _bintree_insert(tree, &tree->_root, item);
    
    if(ret)
        _bintree_adjust_size(tree, 1);
    
    return ret;
}

void *bintree_retrieve(struct tree *__restrict tree, void *__restrict key)
{
    return item_data(bintree_retrieve_item(tree, key));
}

struct item *bintree_retrieve_item(struct tree *__restrict tree,
                                   void *__restrict key)
{
    return _bintree_retrieve_item(tree, tree->_root, key);
}

void *bintree_take(struct tree *__restrict tree, void *__restrict key)
{
    struct item *item;
    void *data;
    
    item = bintree_take_item(tree, key);
    if(!item)
        return NULL;
    
    data = item_data(item);
    
    item_delete(item, NULL, tree->_key_delete);
    
    return data;
}

struct item *bintree_take_item(struct tree *__restrict tree, 
                               void *__restrict key)
{
    return _bintree_take(tree, &tree->_root, key);
}

void bintree_delete_item(struct tree *__restrict tree, 
                         void *__restrict key)
{
    struct item *item;
    
    item = bintree_take_item(tree, key);
    
    item_delete(item, tree->_data_delete, tree->_key_delete);
}


struct item *bintree_min_item(struct tree *__restrict tree)
{
    return _bintree_leaf_item(tree, &tree->_root, &item_ref_left, false);
}

struct item *bintree_max_item(struct tree *__restrict tree)
{
    return _bintree_leaf_item(tree, &tree->_root, &item_ref_right, false);
}

void *bintree_min(struct tree *__restrict tree)
{
    return item_data(bintree_min_item(tree));
}

void *bintree_max(struct tree *__restrict tree)
{
    return item_data(bintree_max_item(tree));
}

struct item *bintree_take_min_item(struct tree *__restrict tree)
{
    return _bintree_leaf_item(tree, &tree->_root, &item_ref_left, true);
}

struct item *bintree_take_max_item(struct tree *__restrict tree)
{
    return _bintree_leaf_item(tree, &tree->_root, &item_ref_right, true);
}

void *bintree_take_min(struct tree *__restrict tree)
{
    return _bintree_take_leaf(tree, &bintree_take_min_item);
}

void *bintree_take_max(struct tree *__restrict tree)
{
    return _bintree_take_leaf(tree, &bintree_take_max_item);
}