#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <hash.h>
#include <list.h>
#include <buffer.h>

#include "config.h"

static const char *no_section = "No Section";





struct section {
    char *_name;
    struct hash _data_hash;
};

struct data {
    char *_key;
    char **_data;
    int _size;
};

enum parser_state {
    PARSER_STATE_START,
    PARSER_STATE_FINISHED,
    PARSER_STATE_ERROR,
    PARSER_STATE_HANDLE_SECTION,
    PARSER_STATE_HANDLE_KEY,
    PARSER_STATE_HANDLE_VALUES,
    PARSER_STATE_HANDLE_COMMENT
};

struct config_parser {
    struct config  *config;
    struct buffer  *buf;
    struct section *section;
    struct data    *data;
    char           *key;
    struct list    data_values;
    
    enum parser_state state;
};

static int _config_insert_section(struct config *__restrict config,
                                  struct section *__restrict section);

static struct data *_data_new(char *key, int size)
{
    struct data *d;
    
    d = malloc(sizeof(*d));
    if(!d)
        return NULL;
    
    d->_data = calloc(size, sizeof(*d->_data));
    if(!d->_data) {
        free(d);
        return NULL;
    }
    
    d->_key  = key;
    d->_size = size;
    
    return d;
}

static void *_data_delete(struct data *__restrict d)
{
    while(d->_size--)
        free(d->_data[d->_size]);
    
    free(d->_data);
    free(d->_key);
    free(d);
}

static struct section *_section_new(char *name)
{
    struct section *s;
    int (*key_compare)(const void *, const void *);
    int err;
    
    s = malloc(sizeof(*s));
    if(!s)
        return NULL;
    
    err = hash_init(&s->_data_hash, 0, 0);
    if(err < 0) {
        free(s);
        return NULL;
    }
    
    s->_name = name;
    
    key_compare = &strcmp;
    
    hash_set_data_delete(&s->_data_hash, (void (*)(void *))&_data_delete);
    hash_set_key_delete(&s->_data_hash, NULL);
    
    hash_set_key_length(&s->_data_hash, (size_t (*)(const void *)) &strlen);
    hash_set_key_compare(&s->_data_hash, key_compare);
    
    return s;
}

static void _section_delete(struct section *__restrict s)
{
    hash_destroy(&s->_data_hash);
    free(s);
}

static char *_buffer_strcpy(struct buffer *__restrict buf)
{
    char *s;
    int err;
    
    err = buffer_prepare_write(buf, sizeof(char));
    if(err < 0)
        return NULL;
    
    buffer_write_char(buf, '\0');
    
    s = buffer_data(buf);
    
    return strdup(s);
}

static struct config_parser *_config_parser_new(struct config *config)
{
    struct config_parser *parser;
    
    parser = malloc(sizeof(*parser));
    if(!parser)
        return NULL;
    
    memset(parser, 0, sizeof(*parser));
    
    parser->buf = buffer_new(512);
    if(!parser->buf) {
        free(parser);
        return NULL;
    }
    
    list_init(&parser->data_values);
    
    parser->config = config;
    parser->state  = PARSER_STATE_START;
    
    return parser;
}

static enum parser_state 
_config_parser_next_state(struct config_parser *__restrict parser)
{
    char c;
    
    while(1) {
        c = getc(parser->config->_file);
        
        switch(c) {
        case '\n':
            break;
        case '[':
            return PARSER_STATE_HANDLE_SECTION;
        case '#':
        case ';':
            return PARSER_STATE_HANDLE_COMMENT;
        case ' ':
        case '\t':
            if(buffer_empty(parser->buf))
                return PARSER_STATE_HANDLE_KEY;
        case EOF:
            return PARSER_STATE_FINISHED;
        default:
            return PARSER_STATE_ERROR;
        }
    }
}

static void _parser_finish_line(struct config_parser *__restrict parser)
{
    char c;
    
    do {
        c = getc(parser->config->_file);
    } while(c != '\n' && c != 'EOF');
}

static int 
_config_parser_handle_section(struct config_parser *__restrict parser)
{
    int err;
    char *s, c, last;
    
    parser->section = NULL;
    last            = EOF;
    
    while(!parser->section) {
        c = getc(parser->config->_file);
        
        switch(c) {
        case ' ':
        case '\t':
            if(buffer_empty(parser->buf))
                break;
            
            if(last == ' ' || last == '\t') 
                break;
            
            err = buffer_prepare_write(parser->buf, sizeof(c));
            if(err < 0)
                return err;
            
            buffer_write_char(parser->buf, c);
            
            break;
        case ']':
            s = _buffer_strcpy(parser->buf);
            if(!s)
                return NULL;
            
            parser->section = _section_new(s);
            if(!parser->section) {
                free(s);
                return -errno;
            }
            
            err = _config_insert_section(parser->config, parser->section);
            if(err < 0) {
                _section_delete(parser->section);
                return err;
            }
            
            _parser_finish_line(parser);
            break;
        default:
            if(!isalnum(c))
                return -EINVAL;
            
            err = buffer_prepare_write(parser->buf, sizeof(c));
            if(err < 0)
                return err;
            
            buffer_write_char(parser->buf, c);
            break;
        }
        
        last = c;
    }
    
    return 0;
}

static int _config_parser_handle_key(struct config_parser *__restrict parser)
{
    char c;
    
    while(!parser->key) {
        c = getc(parser->config->_file);
        
        if(isalnum(c)) {
            
        }
    }
}

static int 
_config_parser_handle_comment(struct config_parser *__restrict parser)
{
    _parser_finish_line(parser);
}

static int _config_parser_parse(struct config_parser *__restrict parser)
{
    int err;
    
    do {
        
        switch(parser->state) {
        case PARSER_STATE_START:
            parser->state = _config_parser_next_state(parser);
            break;
        case PARSER_STATE_HANDLE_SECTION:
            err = _config_parser_handle_section(parser);
            if(err < 0)
                return err;
            
            parser->state = PARSER_STATE_START;
            break;
        case PARSER_STATE_HANDLE_KEY:
            err = _config_parser_handle_key(parser);
            
            parser->state = PARSER_STATE_START;
            break;
        case PARSER_STATE_HANDLE_VALUES:
            
            parser->state = PARSER_STATE_START;
            break;
        case PARSER_STATE_HANDLE_COMMENT:
            _config_parser_handle_comment(parser);
            
            parser->state = PARSER_STATE_START;
            break;
        case PARSER_STATE_ERROR:
        default:
            return -EINVAL;
        }
        
        buffer_clear(parser->buf);
    } while(parser->state != PARSER_STATE_FINISHED);
    
    return 0;
}

static void _config_parser_delete(struct config_parser *__restrict parser)
{
    if(parser->section)
        _section_delete(parser->section);
    
    if(parser->data)
        _data_delete(parser->data);
    
    free(parser->key);
    
    list_destroy(&parser->data_values);
    buffer_delete(parser->buf);
    free(parser);
}

static int _config_parse(struct config *__restrict config)
{
    struct config_parser *parser;
    int err;
    
    parser = _config_parser_new(config);
    if(!parser)
        return -errno;
    
    err = _config_parser_parse(parser);
    
    _config_parser_delete(parser);
    
    return err;
}

static int _config_insert_section(struct config *__restrict config,
                                  struct section *__restrict section)
{
    int err;
    
    err = hash_insert(&config->_hash, section, section->_name);
    if(err < 0)
        return err;
    
    err = list_insert_back(&config->_list, section, NULL);
    if(err < 0) {
        /* returned object is 'section' */
        hash_take(&config->_hash, section->_name);
        return err;
    }
        
    return 0;
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
            
            data = _buffer_strcpy(&buf);
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
            
            key = _buffer_strcpy(&buf);
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

struct config *config_open(const char *__restrict path)
{
    struct config *config;
    FILE *config_file;
    int (*key_compare)(const void *, const void *);
    int err;
    
    
    config_file = fopen(path, "r");
    if(!config_file) {
        err = errno;
        fprintf(stderr,
                "ERROR: opening config %s failed - %s\n",
                path, strerror(err));
        goto out;
    }
    
    config = malloc(sizeof(*config));
    if(!config) {
        err = errno;
        fprintf(stderr, "ERROR: creating config failed - %s\n", strerror(err));
        goto cleanup1;
    }
    
    err = hash_init(&config->_hash, 0, 0);
   if(err < 0) {
        fprintf(stderr, "ERROR: creating config failed - %s\n", strerror(-err));
        goto cleanup2;
    }
    
    key_compare = &strcmp;
    
    hash_set_key_length(&config->_hash, (size_t (*)(const void *)) &strlen);
    hash_set_data_delete(&config->_hash, &free);
    hash_set_key_delete(&config->_hash, NULL);
    hash_set_key_compare(&config->_hash, key_compare);
    
    err = _config_parse(config, config_file);
    if(err < 0) {
        fprintf(stderr,
                "ERROR: parsing config %s failed - %s\n",
                path, strerror(-err));
        goto cleanup3;
    }
    
    fclose(config_file);
    
    return config;

cleanup3:
    hash_destroy(&config->_hash);
cleanup2:
    free(config);
cleanup1:
    fclose(config_file);
out:
    return NULL;
}

char *config_lookup(struct config *__restrict config, const char *__restrict key)
{
    return hash_retrieve(config, key);
}

void config_close(struct config *__restrict config)
{
    hash_destroy(&config->_hash);
    free(config);
}
