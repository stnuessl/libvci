#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <hash.h>
#include <list.h>
#include <buffer.h>

#include "config.h"

static char *_stringify_buffer(struct buffer *__restrict buf)
{
    char *s;
    int err;
    
    err = buffer_prepare_write(buf, sizeof(char));
    if(err < 0)
        return NULL;
    
    buffer_write_char(buf, '\0');
    
    s = buffer_data(buf);
    
    s = strdup(s);
    if(!s)
        return NULL;
    
    buffer_clear(buf);
    
    return s;
}

static int _config_parse(config_t *__restrict config, 
                         FILE *__restrict config_file)
{
    struct buffer buf;
    char *data, *key, c;
    int err;
    
    err = buffer_init(&buf, 256);
    if(err < 0)
        return err;
    
    key  = NULL;
    
    do {
        c = getc(config_file);
        
        switch(c) {
        case '#':
            do {
                c = getc(config_file);
            } while(c != '\n');
            
        case '\n':
            if(buffer_empty(&buf))
                break;
            
            if(!key) {
                err = -EINVAL;
                goto out;
            }
            
            data = _stringify_buffer(&buf);
            if(!data)
                goto cleanup_key;
            
            err = hash_insert(config, data, key);
            if(err < 0)
                goto cleanup_data;
            
            key  = NULL;
            break;
        case '=':
            if(key) {
                err = -EINVAL;
                goto cleanup_key;
            }
            
            key = _stringify_buffer(&buf);
            if(!key)
                goto out;
            
            break;
        case ' ':
            break;
        default:
            err = buffer_prepare_write(&buf, sizeof(char));
            if(err < 0)
                goto out;
                
            buffer_write_char(&buf, c);
            break;
        }
        
    } while(c != EOF);
    
    buffer_destroy(&buf);
    free(key);
    
    return 0;

cleanup_data:
    free(data);
cleanup_key:
    free(key);
out:
    buffer_destroy(&buf);
    return err;
}

config_t *config_open(const char *__restrict path)
{
    config_t *config;
    FILE *config_file;
    int err;
    
    config_file = fopen(path, "r");
    if(!config_file) {
        err = errno;
        fprintf(stderr,
                "ERROR: opening config %s failed - %s\n",
                path, strerror(err));
        goto out;
    }
    
    config = hash_new(0, 0);
    if(!config) {
        err = errno;
        fprintf(stderr,
                "ERROR: creating config failed - %s\n",
                strerror(err));
        goto cleanup1;
    }
    
    hash_set_key_length(config, (size_t (*)(const void *)) &strlen);
    hash_set_data_delete(config, &free);
    hash_set_key_delete(config, &free);
    hash_set_key_compare(config, (int (*)(const void *, const void *)) &strcmp);
    
    err = _config_parse(config, config_file);
    if(err < 0) {
        fprintf(stderr,
                "ERROR: parsing config %s failed - %s\n",
                path, strerror(-err));
        goto cleanup2;
    }
    
    fclose(config_file);
    
    return config;

cleanup2:
    hash_delete(config);
cleanup1:
    fclose(config_file);
out:
    return NULL;
}

char *config_read(config_t *__restrict config, const char *__restrict key)
{
    return hash_retrieve(config, key);
}

void config_close(config_t *__restrict config)
{
    hash_delete(config);
}
