#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "map.h"
#include "buffer.h"

#include "config.h"
#include "config_parser.h"

/* max file size 512 kB */
#define CONFIG_MAX_FILE_SIZE (1 << 19)

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
    
    fd = open(config->path, O_RDWR);
    if(fd < 0)
        goto cleanup1;
    
    err = fstat(fd, &stat);
    if(err < 0)
        goto cleanup1;
    
    /* 512 MB */
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
    p->state       = PARSER_STATE_START;
    
    return p;

cleanup2:
    close(fd);
cleanup1:
    free(p);
out:
    return NULL;
}

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
            *parser->p = '\0';
            break;
        case '\n':
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

int config_parser_parse(struct config_parser *__restrict parser)
{
    int err;
    char c, *key, *val;
    
    key = NULL;
    
    for(parser->p = parser->fstart; parser->p < parser->fend; ++parser->p) {
        c = *parser->p;
        
        if(isalnum(c)) {
            if(!key) {
                key = parser_read_key(parser);
                
                if(!key)
                    return -errno;
                
            } else {
                val = parser_read_value(parser);
                if(!val)
                    return -errno;
                
                err = map_insert(&parser->config->map, key, val);
                if(err < 0)
                    return err;
                
                key = NULL;
            }
        } else if(c == '#' || c == ';') {
            parser_skip_line(parser);
        } else if(c == ' ' || c == '\t' || c == '\n') {
            continue;
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