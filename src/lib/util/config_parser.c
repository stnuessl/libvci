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
#include "config_key.h"

static const char *_buffer_string(struct buffer *__restrict buf)
{
    int err;
    
    err = buffer_prepare_write(buf, sizeof(char));
    if(err < 0)
        return NULL;
    
    buffer_write_char(buf, '\0');
    
    return buffer_data(buf);
}

static inline 
bool _parser_char_readable(struct config_parser *__restrict parser)
{
    return parser->fpos < parser->file + parser->fsize;
}

static enum parser_state 
_config_parser_next_state(struct config_parser *__restrict parser)
{
    char c;
    
    while(1) {
        if(!_parser_char_readable(parser))
            return PARSER_STATE_FINISHED;
        
        c = *parser->fpos++;
        
        switch(c) {
            case '\n':
            case ' ':
            case '\t':
            case '=':
                break;
            case '[':
                return PARSER_STATE_HANDLE_SECTION;
            case '#':
            case ';':
                return PARSER_STATE_HANDLE_COMMENT;
            default:
                --parser->fpos;
                
                if(!isalnum(c))
                    return PARSER_STATE_ERROR;
                                
                if(parser->key)
                    return PARSER_STATE_HANDLE_VALUES;
                else
                    return PARSER_STATE_HANDLE_KEY;
        }
    }
}

static void _parser_finish_line(struct config_parser *__restrict parser)
{
    while(_parser_char_readable(parser)) {
        if(*parser->fpos++ == '\n')
            break;
    }
}

static int 
_config_parser_handle_section(struct config_parser *__restrict parser)
{
    int err;
    const char *s;
    char c;

    free(parser->section);
    parser->section = NULL;
    
    while(_parser_char_readable(parser) && !parser->section) {
        c = *parser->fpos++;
        
        switch(c) {
        case ' ':
        case '\t':
            if(buffer_empty(parser->buf))
                break;
        case ']':
            s = _buffer_string(parser->buf);
            if(!s)
                return -errno;
            
            parser->section = strdup(s);
            if(!parser->section)
                return -errno;
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
    }
    
    return (parser->section) ? 0 : -EINVAL;
}

static int _config_parser_handle_key(struct config_parser *__restrict parser)
{
    int err;
    char c;
    const char *s;
    
    while(_parser_char_readable(parser) && !parser->key) {
        c = *parser->fpos++;
        
        switch(c) {
        case ' ':
        case '=':
        case '\t':
        case '\n':
            s = _buffer_string(parser->buf);
            if(!s)
                return -errno;
            
            parser->key = config_key_merge(parser->section, s);
            break;
        default:
            if(!isalnum(c))
                return -EINVAL;
            
            err = buffer_prepare_write(parser->buf, sizeof(c));
            if(err < 0)
                return -errno;
            
            buffer_write_char(parser->buf, c);
            break;
        }
    }
    
    return 0;
}

static int _config_parser_handle_values(struct config_parser *__restrict parser)
{
    int err;
    char *start, *end, *values, c;
    const char *s;
    size_t diff;
    
    start = parser->fpos;
    
    while(_parser_char_readable(parser)) {
        if(*parser->fpos == '\n')
            break;
        
        parser->fpos += 1;
    }
    
    end = parser->fpos;
    
    do {
        c = *end--;
    } while(c == ' ' || c == '\t');
    
    diff = end - start + 1;
    
    err = buffer_prepare_write(parser->buf, diff);
    if(err < 0)
        return err;
    
    buffer_write(parser->buf, start, diff);
    
    s = _buffer_string(parser->buf);
    if(!s)
        return -errno;
    
    values = strdup(s);
    if(!values)
        return -errno;
    
    err = map_insert(&parser->config->map, (void *)parser->key, values);
    if(err < 0) {
        free(values);
        return -errno;
    }

    parser->key = 0;
    
    return 0;
}

static void 
_config_parser_handle_comment(struct config_parser *__restrict parser)
{
    _parser_finish_line(parser);
}

struct config_parser *config_parser_new(struct config *config)
{
    struct config_parser *p;
    struct stat file_info;
    int fd, err;
    
    p = malloc(sizeof(*p));
    if(!p)
        goto out;
    
    memset(p, 0, sizeof(*p));
    
    p->buf = buffer_new(512);
    if(!p->buf)
        goto cleanup1;
    
    p->section = strdup(_NO_SECTION_);
    if(!p->section)
        goto cleanup2;
    
    fd = open(config->path, O_RDWR);
    if(fd < 0)
        goto cleanup3;
    
    err = fstat(fd, &file_info);
    if(err < 0)
        goto cleanup4;
    
    p->fsize = file_info.st_size;
    
    /* 512 MB */
    if(p->fsize > (1 << 29)) {
        errno = EINVAL;
        goto cleanup4;
    }
    
    p->file = mmap(NULL, p->fsize, PROT_READ, MAP_SHARED, fd, 0);
    if(!p->file)
        goto cleanup4;
    
    close(fd);
    
    p->config = config;
    p->state  = PARSER_STATE_START;
    p->fpos   = p->file;
    
    return p;

cleanup4:
    close(fd);
cleanup3:
    free(p->section);
cleanup2:
    buffer_delete(p->buf);
cleanup1:
    free(p);
out:
    return NULL;
}

int config_parser_parse(struct config_parser *__restrict parser)
{
    int err;
    
    err =  0;
    
    do {
        parser->state = _config_parser_next_state(parser);
        
        switch(parser->state) {
            case PARSER_STATE_HANDLE_SECTION:
                err = _config_parser_handle_section(parser);
                break;
            case PARSER_STATE_HANDLE_KEY:
                err = _config_parser_handle_key(parser);
                break;
            case PARSER_STATE_HANDLE_VALUES:
                err = _config_parser_handle_values(parser);
                break;
            case PARSER_STATE_HANDLE_COMMENT:
                _config_parser_handle_comment(parser);
                break;
            case PARSER_STATE_FINISHED:
                break;
            case PARSER_STATE_ERROR:
            default:
                err = -EINVAL;
                break;
        }
        
        buffer_clear(parser->buf);
    } while(parser->state != PARSER_STATE_FINISHED && err == 0);
    
    return err;
}

void config_parser_delete(struct config_parser *__restrict parser)
{
    munmap(parser->file, parser->fsize);
    
    free(parser->section);
    
    buffer_delete(parser->buf);
    free(parser);
}