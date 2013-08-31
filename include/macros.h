#ifndef _MACROS_H_
#define _MACROS_H_

/* some useful macro definitions */

#define min(a, b)                                                              \
    ((a) < (b)) ? (a) : (b)
    
#define max(a, b)                                                              \
    ((a) > (b)) ? (a) : (b)
    
#define ARRAY_SIZE(a)                                                        \
    (sizeof((a)) / sizeof(*(a)))

#define ALIGN(x, a)                                                            \
    __ALIGN_MASK(x, (typeof(x))(a)-1)

#define __ALIGN_MASK(x, mask)                                                  \
    (((x) + (mask)) & ~(mask))

#define container_of(ptr, type, member)                                        \
    (type *)(((char *) ptr) - offsetof(type, member))

#define likely(x)                                                              \
    __builtin_expect(!!(x), 1)
    
#define unlikely(x)                                                            \
    __builtin_expect(!!(x), 0)

#endif /* _MACROS_H_ */