#include "config.h"
#include "key.h"


int key_compare(const void *m1, const void *m2)
{
    return (unsigned long) m1 - (unsigned long) m2;
}

unsigned int key_hash(const void *key)
{
    unsigned long k;
    const char *buf;
    size_t size;
    unsigned int hval;
    
    k = (unsigned long) key;
    buf = (const char *)&k;
    size = sizeof(k);
    hval = 1;
    
    
    while(size--) {
        hval += *buf++;
        hval += (hval << 10);
        hval ^= (hval >> 6);
        hval &= 0x0fffffff;
    }
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    return hval;
}

unsigned long key_merge(const char *__restrict section, 
                        const char *__restrict key)
{
    unsigned long ret;
    char *buf;
    const char *s, *k;
    size_t size;
    
    buf = (char *)&ret;
    size = sizeof(ret);
    
    s = section;
    k = key;
    
    while(size--) {
        if(*s == '\0')
            s = section;
        if(*k == '\0')
            k = key;
        
        ((char *)buf)[size] = *s++ ^ *k++;
    }
    
    return ret;
}