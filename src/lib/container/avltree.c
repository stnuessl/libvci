/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include "avltree.h"
#include "macro.h"

static inline int _avlnode_get_height(const struct avlnode *__restrict node)
{
    return (node) ? node->height : -1;
}

static void _avlnode_rotate_left(struct avlnode **node)
{
    struct avlnode *tmp;
    int left_height, right_height;
    
    tmp            = (*node)->right;
    (*node)->right = tmp->left;
    tmp->left      = *node;
    
    left_height  = _avlnode_get_height((*node)->left);
    right_height = _avlnode_get_height((*node)->right);
    
    (*node)->height = max(left_height, right_height) + 1;
    
    left_height  = _avlnode_get_height(tmp->left);
    right_height = _avlnode_get_height(tmp->right);
    
    tmp->height = max(left_height, right_height) + 1;

    tmp->parent     = (*node)->parent;
    (*node)->parent = tmp;
    
    if((*node)->right)
        (*node)->right->parent = *node;
    
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
    
    tmp->parent     = (*node)->parent;
    (*node)->parent = tmp;
    
    if((*node)->left)
        (*node)->left->parent = *node;
    
    *node = tmp;
}

static int _avlnode_get_balance(struct avlnode *__restrict node)
{
    int left_height, right_height;
    
    if(!node)
        return 0;
    
    left_height  = _avlnode_get_height(node->left);
    right_height = _avlnode_get_height(node->right);
    
    return right_height - left_height;
}

static struct avlnode **
_avlnode_child_reference(struct avlnode *__restrict node)
{
    /* get reference of parents pointer to 'node' */
    if(node->parent->left == node)
        return &node->parent->left;
    else
        return &node->parent->right;
}

static void _avltree_rebalance_nodes(struct avltree *__restrict tree,
                                     struct avlnode *node)
{
    struct avlnode **node_ref;;
    int left_height, right_height, balance;
    
    while(node) {
        left_height  = _avlnode_get_height(node->left);
        right_height = _avlnode_get_height(node->right);
        
        node->height = max(left_height, right_height) + 1;
        
        if(node->parent)
            node_ref = _avlnode_child_reference(node);
        else
            node_ref = &tree->root;
        
        balance = _avlnode_get_balance(node);
        
        if(balance == -2) {
            if(_avlnode_get_balance(node->left) < 1) {
                _avlnode_rotate_right(node_ref);
            } else {
                _avlnode_rotate_left(&node->left);
                _avlnode_rotate_right(node_ref);
            }
        } else if(balance == 2) {
            if(_avlnode_get_balance(node->right) > -1) {
                _avlnode_rotate_left(node_ref);
            } else {
                _avlnode_rotate_right(&node->right);
                _avlnode_rotate_left(node_ref);
            }
        }
        
        node = node->parent;
    }
}

static struct avlnode *_avlnode_take_min(struct avlnode **node)
{
    struct avlnode *min;
    
    while((*node)->left)
        node = &(*node)->left;
    
    min = *node;
    
    if((*node)->right) {
        (*node)->right->parent = (*node)->parent;
        *node = (*node)->right;
    } else {
        *node = NULL;
    }
    
    return min;
}

static struct avlnode *_avltree_take_node(struct avltree *__restrict tree,
                                          struct avlnode **node)
{
    struct avlnode *tmp, *min;
    
    tmp = *node;
    
    if((*node)->left && (*node)->right) {
        /* get reference on minimum node in right 'half'-tree */

        min = _avlnode_take_min(&(*node)->right);

        min->left   = (*node)->left;
        min->right  = (*node)->right;
        min->parent = (*node)->parent;
        min->height = (*node)->height;
        
        *node = min;
    } else if((*node)->left) {
        (*node)->left->parent = (*node)->parent;
        *node = (*node)->left;
    } else if((*node)->right) {
        (*node)->right->parent = (*node)->parent;
        *node = (*node)->right;
    } else {
        *node = NULL;
    }
    
    tree->size -= 1;
    
    return tmp;
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
    struct avlnode *node;
    
    if(!tree->data_delete)
        return;
    
    while(tree->root) {
        node = _avltree_take_node(tree, &tree->root);
        
        tree->data_delete(node);
    }
}

int avltree_insert(struct avltree *__restrict tree,
                   struct avlnode *node, 
                   const void *key)
{
    struct avlnode **root, *parent;
    int res;
    
    root   = &tree->root;
    parent = NULL;
    
    while(*root) {
        res = tree->key_compare(key, (*root)->key);
        
        parent = *root;
        
        if(res < 0)
            root = &(*root)->left;
        else if(res > 0)
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
    tree->size += 1;
    
    _avltree_rebalance_nodes(tree, parent);
    
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
    struct avlnode **root, *ret;
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
    
    ret = _avltree_take_node(tree, root);
    
    _avltree_rebalance_nodes(tree, ret->parent);
    
    return ret;
}

bool avltree_contains(struct avltree *__restrict tree,
                      const void *key)
{
    return avltree_retrieve(tree, key) != NULL;
}

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
    struct avlnode **node, *min;
    
    node = &tree->root;
    
    if(!*node)
        return NULL;
    
    while((*node)->left)
        node = &(*node)->left;
    
    min = _avltree_take_node(tree, node);
    
    _avltree_rebalance_nodes(tree, min->parent);
    
    return min;
}

struct avlnode *avltree_take_max(struct avltree *__restrict tree)
{
    struct avlnode **node, *max;
    
    node = &tree->root;
    
    if(!*node)
        return NULL;
    
    while((*node)->right)
        node = &(*node)->right;
    
    max = _avltree_take_node(tree, node);
    
    _avltree_rebalance_nodes(tree, max->parent);
    
    return max;
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

struct avlnode *avlnode_postorder_first(struct avlnode *node)
{
    while(1) {
        if(node->left)
            node = node->left;
        else if(node->right)
            node = node->right;
        else
            break;
    }
    
    return node;
}

struct avlnode *avlnode_postorder_next(struct avlnode *node)
{
    if(unlikely(!node->parent))
        return NULL;
        
    if(node->parent->right && node->parent->left == node)
        return avlnode_postorder_first(node->parent->right);
    
    return node->parent;
}