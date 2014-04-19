#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "map.h"
#include "list.h"
#include "buffer.h"
#include "macro.h"
#include "config.h"
#include "config_parser.h"

static int string_compare(const void *a, const void *b)
{
    return strcmp(a, b);
}

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
    if(!config->path) {
        err = -errno;
        goto out;
    }
    
    err = map_init(&config->map, 0, &string_compare, &hash_string);
    if(err < 0) 
        goto cleanup1;
    
    return 0;

cleanup1:
    free(config->path);
out:    
    return err;
}

void config_destroy(struct config *__restrict config)
{
    map_destroy(&config->map);
    free(config->path);
    free(config->mem);
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
                    const char *__restrict key)
{
    return map_retrieve(&config->map, key);
}

int config_add_value(struct config *__restrict config, 
                     const char *key,
                     char *value)
{
    return map_insert(&config->map, key, value);
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

int config_save(struct config *__restrict config)
{
    return config_save_to_file(config, config->path);
}

int config_save_to_file(struct config *__restrict config, 
                        const char *__restrict path)
{
    FILE *f;
    struct entry *e;
    const char *key;
    char *val;
    
    f = fopen(path, "w");
    if(!f)
        return -errno;
    
    config_for_each(config, e) {
        key = entry_key(e);
        val = entry_data(e);
        
        fprintf(f, "%s = %s\n", key, val);
    }
    
    fclose(f);
    
    return 0;
}