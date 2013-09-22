#ifndef _KEYS_H_
#define _KEYS_H_

#include <stdio.h>

int key_compare(const void *m1, const void *m2);

void key_merge(void *__restrict buf,
                size_t size,
                const char *__restrict section,
                const char *__restrict key);

#endif /* _KEYS_H_ */