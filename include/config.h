#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdbool.h>

#include "hash.h"
#include "list.h"

struct config {
    FILE *_file;
    struct hash _hash;
    struct list _list;
};

struct config *config_open(const char *__restrict path, bool writable);

char *config_get(struct config *__restrict config, const char *__restrict key);

void config_close(struct config  *__restrict config);

#endif /* _CONFIG_H_ */  