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

#ifndef _AVLTREE_H_
#define _AVLTREE_H_

#include <stdbool.h>

struct avlnode {
    const void *key;
    struct avlnode *parent;
    struct avlnode *left;
    struct avlnode *right;
    
    unsigned int height;
};

struct avltree {
    struct avlnode *root;
    
    int (*key_compare)(const void *, const void *);
    void (*data_delete)(struct avlnode *);
    
    unsigned int size;
};

struct avltree *avltree_new(int (*key_compare)(const void *, const void *));

void avltree_delete(struct avltree *__restrict tree);

void avltree_init(struct avltree *__restrict avltree, 
                  int (*key_compare)(const void *, const void *));

void avltree_destroy(struct avltree *__restrict tree);

int avltree_insert(struct avltree *__restrict tree,
                   struct avlnode *node, 
                   const void *key);

struct avlnode *avltree_retrieve(struct avltree *__restrict tree, 
                                 const void *key);

struct avlnode *avltree_take(struct avltree *__restrict tree, 
                             const void *key);

bool avltree_contains(struct avltree *__restrict tree,
                      const void *key);

struct avlnode *avltree_min(struct avltree *__restrict tree);

struct avlnode *avltree_max(struct avltree *__restrict tree);

struct avlnode *avltree_take_min(struct avltree *__restrict tree);

struct avlnode *avltree_take_max(struct avltree *__restrict tree);

unsigned int avltree_size(const struct avltree *__restrict tree);

bool avltree_empty(const struct avltree *__restrict tree);

void avltree_set_key_compare(struct avltree *__restrict tree,
                             int (*key_compare)(const void *, const void *));

int (*avltree_key_compare(struct avltree *__restrict avltree))
                         (const void *, const void *);

void avltree_set_data_delete(struct avltree *__restrict tree,
                             void (*data_delete)(struct avlnode *));

void (*avltree_data_delete(struct avltree *__restrict tree))
                           (struct avlnode *);

struct avlnode *avlnode_postorder_first(struct avlnode *node);

struct avlnode *avlnode_postorder_next(struct avlnode *node);

#define avltree_for_each_postorder(tree, node)                                 \
    for((node) = avlnode_postorder_first((tree)->root);                        \
        (node);                                                                \
        (node) = avlnode_postorder_next((node)))

#define avltree_for_each_postorder_safe(tree, node, safe)                      \
    for((node) = avlnode_postorder_first((tree)->root),                        \
        (safe) = avlnode_postorder_next((node));                               \
        (node);                                                                \
        (node) = (safe), (safe) = avlnode_postorder_next((node)))

#endif /* _AVLTREE_H_ */