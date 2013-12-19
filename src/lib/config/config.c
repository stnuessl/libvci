#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#include <map.h>
#include <list.h>
#include <buffer.h>
#include <macros.h>

#include "config.h"
#include "config_parser.h"
#include "key.h"


struct config *config_new(const char *__restrict path)
{
    struct config *config;
    int err;
    
    config = malloc(sizeof(*config));
    if(!config)
        return NULL;
    
    err = config_init(config, path);
    if(err < 0) {
        free(config);
        return NULL;
    }
    
    return config;
}

void config_delete(struct config *__restrict config)
{
    config_destroy(config);
    free(config);
}

int config_init(struct config *__restrict config, const char *__restrict path)
{
    int err;
    
    config->path = strdup(path);
    if(!config->path)
        return -errno;
    
    err = map_init(&config->map, 0, &key_compare, &key_hash);
    if(err < 0) {
        free(config->path);
        return err;
    }
    
    map_set_data_delete(&config->map, &free);
    
    return 0;
}

void config_destroy(struct config *__restrict config)
{
    free(config->path);
    map_destroy(&config->map);
}

int config_parse(struct config *__restrict config)
{
    struct config_parser *parser;
    int err;
    
    parser = config_parser_new(config);
    if(!parser)
        return -errno;
    
    err = config_parser_parse(parser);
    
    config_parser_delete(parser);
    
    return err;
}

const char *config_value(struct config *__restrict config, 
                    const char *__restrict section,
                    const char *__restrict key)
{
    uint64_t merged_key;
    
    if(!section)
        section = _NO_SECTION_;
    
    merged_key = key_merge(section, key);
    
    return map_retrieve(&config->map, (void *) merged_key);
}

int config_set_path(struct config *__restrict config, 
                     const char *__restrict path)
{
    free(config->path);
   
    config->path = strdup(path);
    if(!config->path)
        return -errno;
    
    return 0;
}

const char *config_path(struct config *__restrict config)
{
    return config->path;
}