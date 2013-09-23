#include <stdint.h>

#include "config.h"
#include "key.h"


int key_compare(const void *m1, const void *m2)
{
    return *(uint64_t *)m1 - *(uint64_t *)m2;
}

void key_merge(void *__restrict buf,
                size_t size,
                const char *__restrict section,
                const char *__restrict key)
{
    const char *s, *k;
    
    s = section;
    k = key;
    
    while(size--) {
        if(*s == '\0')
            s = section;
        if(*k == '\0')
            k = key;
        
        ((char *)buf)[size] = *s++ ^ *k++;
    }
}