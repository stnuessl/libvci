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

#include "map.h"
#include "hash.h"
#include "compare.h"
#include "list.h"
#include "buffer.h"
#include "macro.h"
#include "config.h"
#include "config_parser.h"

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
    
    err = map_init(&config->map, 0, &compare_string, &hash_string);
    if(err < 0) 
        goto cleanup1;
    
    config->mem = NULL;
    
    return 0;

cleanup1:
    free(config->path);
out:    
    return err;
}

void config_destroy(struct config *__restrict config)
{
    free(config->mem);
    map_destroy(&config->map);
    free(config->path);
}

int config_parse(struct config *__restrict config)
{
    struct config_parser *parser;
    int err;
    
    if(!map_empty(&config->map))
        map_clear(&config->map);
    
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
    return map_retrieve(&config->map, key);
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