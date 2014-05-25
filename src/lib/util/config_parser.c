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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "map.h"
#include "buffer.h"

#include "config.h"
#include "config_parser.h"

/* max file size 512 kB */
#define CONFIG_MAX_FILE_SIZE (1 << 19)

static void parser_skip_line(struct config_parser *__restrict parser)
{
    char c;
    
    for(; parser->p < parser->fend; ++parser->p) {
        c = *parser->p;
        
        if(c == '\n' || c == EOF)
            break;
    }
}


static char *parser_read_key(struct config_parser *__restrict parser)
{
    char *start, c;
    
    for(start = parser->p; parser->p < parser->fend; ++parser->p) {
        c = *parser->p;
        
        switch(c) {
            case ' ':
            case '\t':
            case '\0':
                *parser->p = '\0';
                break;
            case '=':
                *parser->p = '\0';
                return start;
            default:
                if(!isalnum(c))
                    goto out;
                
                break;
        }
    }
    
out:
    errno = EINVAL;
    return NULL;
}

static char *parser_read_value(struct config_parser *__restrict parser)
{
    char *start, c;
    
    for(start = parser->p; parser->p < parser->fend; ++parser->p) {
        c = *parser->p;
        
        switch(c) {
            case ' ':
            case '\t':
            case '\0':
                *parser->p = '\0';
                break;
            case '\n':
                *parser->p = '\0';
                return start;
            default:
                break;
        }
    }
    
    errno = EINVAL;
    return NULL;
}

struct config_parser *config_parser_new(struct config *config)
{
    struct config_parser *p;
    struct stat stat;
    int fd, err;
    ssize_t n;
    
    p = malloc(sizeof(*p));
    if(!p)
        goto out;
    
    memset(p, 0, sizeof(*p));
    
    fd = open(config->path, O_RDONLY);
    if(fd < 0)
        goto cleanup1;
    
    err = fstat(fd, &stat);
    if(err < 0)
        goto cleanup1;
    
    if(stat.st_size > CONFIG_MAX_FILE_SIZE) {
        errno = EINVAL;
        goto cleanup1;
    }
    
    p->fstart = malloc(stat.st_size);
    if(!p->fstart)
        goto cleanup1;
    
    n = read(fd, p->fstart, stat.st_size);
    if(n < 0) {
        err = -errno;
        goto cleanup2;
    }
    
    close(fd);
    
    p->fend        = p->fstart + stat.st_size;
    p->config      = config;
    p->config->mem = p->fstart;
    
    return p;

cleanup2:
    close(fd);
cleanup1:
    free(p);
out:
    return NULL;
}

int config_parser_parse(struct config_parser *__restrict parser)
{
    struct config_handle *handle;
    int err;
    char c, *key, *val;
    
    key = NULL;
    
    for(parser->p = parser->fstart; parser->p < parser->fend; ++parser->p) {
        c = *parser->p;
        
        if(isspace(c))
            continue;
        
        if(c == '#' || c == ';') {
            parser_skip_line(parser);
            
        } else if(isalnum(c) && !key) {
            key = parser_read_key(parser);
                
            if(!key)
                return -errno;
            
        } else if(isgraph(c) && key) {
            val = parser_read_value(parser);
            if(!val)
                return -errno;
            
            err = map_insert(&parser->config->key_map, key, val);
            if(err < 0)
                return err;
            
            handle = map_retrieve(&parser->config->handle_map, key);
            if(handle)
                handle->func(key, val, handle->arg);
            
            key = NULL;
        } else {
            return -EINVAL;
        }
    }
    
    return 0;
}

void config_parser_delete(struct config_parser *__restrict parser)
{
    free(parser);
}