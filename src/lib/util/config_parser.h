#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include "config.h"

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
    char *fstart;
    char *fend;
    char *p;
    char *key;
    
    enum parser_state state;
};

struct config_parser *config_parser_new(struct config *config);

int config_parser_parse(struct config_parser *__restrict parser);

void config_parser_delete(struct config_parser *__restrict parser);

#endif /* _CONFIG_PARSER_H_ */