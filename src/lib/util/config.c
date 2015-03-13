/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "map.h"
#include "hash.h"
#include "compare.h"
#include "list.h"
#include "buffer.h"
#include "macro.h"
#include "config.h"
#include "config_parser.h"
#include "filesystem.h"

struct config *config_new(const char *__restrict path,
                          const char *__restrict conf_txt)
{
    struct config *config;
    int err;
    
    config = malloc(sizeof(*config));
    if(!config)
        return NULL;
    
    err = config_init(config, path, conf_txt);
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

int config_init(struct config *__restrict config, 
                const char *__restrict path,
                const char *__restrict conf_txt)
{
    const struct map_config map_conf = {
        .size           = MAP_DEFAULT_SIZE,
        .lower_bound    = MAP_DEFAULT_LOWER_BOUND,
        .upper_bound    = MAP_DEFAULT_UPPER_BOUND,
        .static_size    = false,
        .key_compare    = &compare_string,
        .key_hash       = &hash_string,
    };
    struct stat st;
    int fd, err;
    
    config->path = strdup(path);
    if(!config->path) 
        return -errno;
    
    err = stat(config->path, &st);
    if (err < 0) {
        if (errno != ENOENT) {
            err = -errno;
            goto cleanup1;
        }
        
        err = path_create(config->path, 0755);
        if (err < 0)
            goto cleanup1;
        
        fd = open(config->path, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (fd < 0) {
            if (errno != EEXIST) {
                err = -errno;
                goto cleanup1;
            }
        }
        
        if (conf_txt)
            write(fd, conf_txt, strlen(conf_txt));
        
        close(fd);
    }
    
    err = map_init(&config->key_map, &map_conf);
    if(err < 0) 
        goto cleanup1;
    
    err = map_init(&config->handle_map, &map_conf);
    if(err < 0)
        goto cleanup2;
    
    config->mem = NULL;
    
    return 0;

cleanup2:
    map_destroy(&config->key_map);
cleanup1:
    free(config->path);
    
    return err;
}

void config_destroy(struct config *__restrict config)
{
    free(config->mem);
    map_destroy(&config->handle_map);
    map_destroy(&config->key_map);
    free(config->path);
}

int config_parse(struct config *__restrict config)
{
    struct config_parser *parser;
    int err;
    
    if(!map_empty(&config->key_map))
        map_clear(&config->key_map);
    
    free(config->mem);
    
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
    return map_retrieve(&config->key_map, key);
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

int config_insert_handle(struct config *__restrict config, 
                         struct config_handle *handle)
{
    return map_insert(&config->handle_map, handle->key, handle);
}

struct config_handle *config_take_handle(struct config *__restrict config,
                                         struct config_handle *handle)
{
    return map_take(&config->handle_map, handle->key);
}