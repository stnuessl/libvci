#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include "avltree.h"
#include "macros.h"

static inline int _avlnode_get_height(const struct avlnode *node)
{
    return (node) ? node->height : -1;
}

static void _avlnode_rotate_left(struct avlnode **node)
{
    struct avlnode *tmp;
    int left_height, right_height;
    
    tmp            = (*node)->right;
    (*node)->right = tmp->left;
    tmp->left     = *node;
    
    left_height  = _avlnode_get_height((*node)->left);
    right_height = _avlnode_get_height((*node)->right);
    
    (*node)->height = max(left_height, right_height) + 1;
    
    left_height  = _avlnode_get_height(tmp->left);
    right_height = _avlnode_get_height(tmp->right);
    
    tmp->height = max(left_height, right_height) + 1;
    
    tmp->parent = (*node)->parent;
    (*node)->parent = tmp;
    
    *node = tmp;
}

static void _avlnode_rotate_right(struct avlnode **node)
{
    struct avlnode *tmp;
    int left_height, right_height;
    
    tmp           = (*node)->left;
    (*node)->left = tmp->right;
    tmp->right    = *node;
    
    left_height  = _avlnode_get_height((*node)->left);
    right_height = _avlnode_get_height((*node)->right);
    
    (*node)->height = max(left_height, right_height) + 1;
    
    left_height  = _avlnode_get_height(tmp->left);
    right_height = _avlnode_get_height(tmp->right);
    
    tmp->height = max(left_height, right_height) + 1;
    
    tmp->parent = (*node)->parent;
    (*node)->parent = tmp;
    
    *node = tmp;
}

static int _avlnode_get_balance(struct avlnode *node)
{
    int left_height, right_height;
    
    if(!node)
        return 0;
    
    left_height  = _avlnode_get_height(node->left);
    right_height = _avlnode_get_height(node->right);
    
    return right_height - left_height;
}

static void _avlnode_balance(struct avlnode **node)
{
    int left_height, right_height, balance;
    
    left_height  = _avlnode_get_height((*node)->left);
    right_height = _avlnode_get_height((*node)->right);
    
    (*node)->height = max(left_height, right_height) + 1;
    
    balance = _avlnode_get_balance(*node);
    
    if(balance == -2) {
        if(_avlnode_get_balance((*node)->left) < 1) {
            _avlnode_rotate_right(node);
        } else {
            _avlnode_rotate_left(&(*node)->left);
            _avlnode_rotate_right(node);
        }
    } else if(balance == 2) {
        if(_avlnode_get_balance((*node)->right) > -1) {
            _avlnode_rotate_left(node);
        } else {
            _avlnode_rotate_right(&(*node)->right);
            _avlnode_rotate_left(node);
        }
    }
}


static void _avltree_take_node(struct avltree *__restrict tree,
                               struct avlnode **node)
{
    struct avlnode **min, *tmp;
    
    if((*node)->left && (*node)->right) {
        /* get reference on minimum node in right 'half'-tree */
        min = &(*node)->right;
        
        while((*min)->left)
            min = &(*min)->left;
        
        tmp   = *node;
        *node = *min;
        
        (*min)->parent = tmp->parent;
    } else if((*node)->left) {
        (*node)->left->parent = (*node)->parent;
        *node = (*node)->left;
    } else {
        (*node)->right->parent = (*node)->parent;
        *node = (*node)->right;
    }
    
    tree->size -= 1;
}


struct avltree *avltree_new(int (*key_compare)(const void *, const void *))
{
    struct avltree *tree;
    
    tree = malloc(sizeof(*tree));
    if(!tree)
        return NULL;
    
    avltree_init(tree, key_compare);
    
    return tree;
}

void avltree_delete(struct avltree *__restrict tree)
{
    avltree_destroy(tree);
    free(tree);
}

void avltree_init(struct avltree *__restrict tree, 
                  int (*key_compare)(const void *, const void *))
{
    tree->root = NULL;

    tree->key_compare = key_compare;
    tree->data_delete = NULL;
    
    tree->size = 0;
}

void avltree_destroy(struct avltree *__restrict tree)
{
    
}

int avltree_insert(struct avltree *__restrict tree,
                   struct avlnode *node, 
                   const void *key)
{
    struct avlnode **root, *parent;;
    int res;
    
    root   = &tree->root;
    parent = NULL;
    
    while(*root) {
        res = tree->key_compare(key, (*root)->key);
        
        parent = *root;
        
        if(res < 0)
            root = &(*root)->left;
        if(res > 0)
            root = &(*root)->right;
        else
            return -EINVAL;
    }
    
    node->key    = key;
    node->parent = parent;
    node->left   = NULL;
    node->right  = NULL;
    node->height = 0;
    
    *root = node;
    
    root = &node->parent;
    
    while(*root) {
        _avlnode_balance(root);
        root = &(*root)->parent;
    }
    
    return 0;
}

struct avlnode *avltree_retrieve(struct avltree *__restrict tree, 
                                 const void *key)
{
    struct avlnode *node;
    int res;
    
    node = tree->root;
    
    while(node) {
        res = tree->key_compare(key, node->key);
        
        if(res < 0)
            node = node->left;
        else if(res > 0)
            node = node->right;
        else
            break;
    }
    
    return node;
}

struct avlnode *avltree_take(struct avltree *__restrict tree, 
                             const void *key)
{
    struct avlnode **root, **parent;
    int res;
    
    root = &tree->root;
    
    while(*root) {
        res = tree->key_compare(key, (*root)->key);
        
        if(res < 0)
            root = &(*root)->left;
        else if(res > 0)
            root = &(*root)->right;
        else
            break;
    }
    
    if(!*root)
        return NULL;
    
    _avltree_take_node(tree, root);
    
    parent = &(*root)->parent;
    
    while(*parent) {
        _avlnode_balance(parent);
        parent = &(*parent)->parent;
    }
    
    return *root;
}

bool avltree_contains(struct avltree *__restrict tree,
                      const void *key);

struct avlnode *avltree_min(struct avltree *__restrict tree)
{
    struct avlnode *node;
    
    node = tree->root;
    
    if(!node)
        return NULL;
    
    while(node->left)
        node = node->left;
    
    return node;
}

struct avlnode *avltree_max(struct avltree *__restrict tree)
{
    struct avlnode *node;
    
    node = tree->root;
    
    if(!node)
        return NULL;
    
    while(node->right)
        node = node->right;
    
    return node;
}

struct avlnode *avltree_take_min(struct avltree *__restrict tree)
{
    struct avlnode **node, **parent;
    
    node = &tree->root;
    
    if(!*node)
        return NULL;
    
    while((*node)->left)
        node = &(*node)->left;
    
    _avltree_take_node(tree, node);
    
    parent = &(*node)->parent;
    
    while(*parent) {
        _avlnode_balance(parent);
        parent = &(*parent)->parent;
    }
    
    return *node;
}

struct avlnode *avltree_take_max(struct avltree *__restrict tree)
{
    struct avlnode **node, **parent;
    
    node = &tree->root;
    
    if(!*node)
        return NULL;
    
    while((*node)->right)
        node = &(*node)->right;
    
    _avltree_take_node(tree, node);
    
    parent = &(*node)->parent;
    
    while(*parent) {
        _avlnode_balance(parent);
        parent = &(*parent)->parent;
    }
    
    return *node;
}

unsigned int avltree_size(const struct avltree *__restrict tree)
{
    return tree->size;
}

bool avltree_empty(const struct avltree *__restrict tree)
{
    return tree->size == 0;
}

void avltree_set_key_compare(struct avltree *__restrict tree,
                             int (*key_compare)(const void *, const void *))
{
    tree->key_compare = key_compare;
}

int (*avltree_key_compare(struct avltree *__restrict tree))
                         (const void *, const void *)
{
    return tree->key_compare;
}

void avltree_set_data_delete(struct avltree *__restrict tree,
                             void (*data_delete)(struct avlnode *))
{
    tree->data_delete = data_delete;
}

void (*avltree_data_delete(struct avltree *__restrict tree))
                          (struct avlnode *)
{
    return tree->data_delete;
}