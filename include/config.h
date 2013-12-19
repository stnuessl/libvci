#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdbool.h>

#include "map.h"
#include "list.h"

struct config {
    char *path;
    struct map map;
};

struct config *config_new(const char *__restrict path);

void config_delete(struct config *__restrict config);

int config_init(struct config *__restrict config, const char *__restrict path);

void config_destroy(struct config *__restrict config);

int config_parse(struct config *__restrict config);

const char *config_value(struct config *__restrict config, 
                         const char *__restrict section,
                         const char *__restrict key);

int config_set_path(struct config *__restrict config, 
                     const char *__restrict path);

const char *config_path(struct config *__restrict config);

#endif /* _CONFIG_H_ */  