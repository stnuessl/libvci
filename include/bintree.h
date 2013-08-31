#ifndef _BINTREE_H_
#define _BINTREE_H_

#include "item.h"
#include "tree.h"

bool bintree_insert(struct tree *__restrict tree, 
                    void *__restrict data, 
                    void *__restrict key);

bool bintree_insert_item(struct tree *__restrict tree, 
                         struct item *__restrict item);

void *bintree_retrieve(struct tree *__restrict tree, void *__restrict key);

struct item *bintree_retrieve_item(struct tree *__restrict tree, 
                                   void *__restrict key);

void *bintree_take(struct tree *__restrict tree, void *__restrict key);

struct item *bintree_take_item(struct tree *__restrict tree, 
                               void *__restrict key);

void bintree_delete_item(struct tree *__restrict tree, 
                         void *__restrict key);

struct item *bintree_min_item(struct tree *__restrict tree);

struct item *bintree_max_item(struct tree *__restrict tree);

void *bintree_min(struct tree *__restrict tree);

void *bintree_max(struct tree *__restrict tree);

struct item *bintree_take_min_item(struct tree *__restrict tree);

struct item *bintree_take_max_item(struct tree *__restrict tree);

void *bintree_take_min(struct tree *__restrict tree);

void *bintree_take_max(struct tree *__restrict tree);

#endif /* _BINTREE_H_ */