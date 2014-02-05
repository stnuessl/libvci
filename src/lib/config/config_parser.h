#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include "config.h"

#define _NO_SECTION_ "__NO_SECTION__"

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
    struct config *config;
    struct buffer *buf;
    char *section;
    unsigned long key;
    char *file;
    size_t fsize;
    char *fpos;
    
    enum parser_state state;
};

struct config_parser *config_parser_new(struct config *config);

int config_parser_parse(struct config_parser *__restrict parser);

void config_parser_delete(struct config_parser *__restrict parser);

#endif /* _CONFIG_PARSER_H_ */