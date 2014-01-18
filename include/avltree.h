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

#endif /* _AVLTREE_H_ */