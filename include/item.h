
#ifndef _ITEM_H_
#define _ITEM_H_

struct item {
    void *_data;
    void *_key;
    
    struct item *_items[2];
};

struct item *item_new(void *__restrict data, void *__restrict key);

struct item *item_clone(struct item *__restrict item, 
                        void *(*data_clone)(void *), 
                        void *(*key_clone)(void *),
                        void (*data_delete)(void *),
                        void (*key_delete)(void *));

void item_delete(struct item *__restrict item, 
                 void (*data_delete)(void *),
                 void (*key_delete)(void *));

void item_swap(struct item *__restrict a, struct item *__restrict b);

struct item *item_nth_next(struct item *__restrict item, int nth);

struct item *item_nth_prev(struct item *__restrict item, int nth);

#define ITEM_DEFINE_SETGET(name, type)                                         \
                                                                               \
inline void item_set_##name(struct item *__restrict item, type name);          \
                                                                               \
inline type item_##name(struct item *__restrict item);

ITEM_DEFINE_SETGET(key, void *)
ITEM_DEFINE_SETGET(data, void *)

#undef ITEM_DEFINE_SETGET

#define ITEM_DEFINE_SETGET_ITEMS(name, which)                                  \
                                                                               \
inline void item_set_##name(struct item *__restrict item, struct item *name);  \
                                                                               \
inline struct item *item_##name(struct item *__restrict item);                 \
                                                                               \
inline struct item **item_ref_##name(struct item *__restrict item);


ITEM_DEFINE_SETGET_ITEMS(prev, 0)
ITEM_DEFINE_SETGET_ITEMS(next, 1)
ITEM_DEFINE_SETGET_ITEMS(left, 0)
ITEM_DEFINE_SETGET_ITEMS(right, 1)

#undef ITEM_DEFINE_SETGET_ITEMS

#endif /* _ITEM_H_ */