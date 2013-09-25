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
    
    config->_path = strdup(path);
    if(!config->_path)
        return -errno;
    
    err = map_init(&config->_map, 0, sizeof(uint64_t));
    if(err < 0) {
        free(config->_path);
        return err;
    }
    
    map_set_data_delete(&config->_map, &free);
    map_set_key_delete(&config->_map, &free);
    map_set_key_compare(&config->_map, &key_compare);
    
    return 0;
}

void config_destroy(struct config *__restrict config)
{
    free(config->_path);
    map_destroy(&config->_map);
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
    
    key_merge(&merged_key, sizeof(merged_key), section, key);
    
    return map_retrieve(&config->_map, &merged_key);
}

int config_set_path(struct config *__restrict config, 
                     const char *__restrict path)
{
    free(config->_path);
   
    config->_path = strdup(path);
    if(!config->_path)
        return -errno;
    
    return 0;
}

const char *config_path(struct config *__restrict config)
{
    return config->_path;
}