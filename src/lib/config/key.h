#ifndef _KEYS_H_
#define _KEYS_H_

int key_compare(const void *m1, const void *m2);

unsigned int key_hash(const void *key);

unsigned long key_merge(const char *__restrict section, 
                        const char *__restrict key);

#endif /* _KEYS_H_ */