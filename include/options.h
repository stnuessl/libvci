/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Steffen Nuessle
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

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdbool.h>

#include "map.h"
#include "vector.h"

enum value_type {
    OPTIONS_BOOL,
    OPTIONS_STRING,
    OPTIONS_MUL_STRING,
    OPTIONS_INT,
    OPTIONS_MUL_INT,
    OPTIONS_DOUBLE,
    OPTIONS_MUL_DOUBLE,
};

struct program_option {
    const char *cmd_flag_long;
    const char *cmd_flag_short;
    enum value_type type;
    void *val;
    const char *description;
};

struct options {
    struct program_option *po;
    unsigned int po_size;
    struct vector unknowns;
};

int options_init(struct options *__restrict o, 
                 struct program_option *po, 
                 unsigned int po_size);

int options_parse(struct options *__restrict o,
                  char ** const argv, 
                  int argc,
                  char **err_msg);

struct vector *options_unknowns(struct options *__restrict o);

void options_help(const struct options *__restrict o,
                  const char *__restrict description,
                  int fd);

void options_destroy(struct options *__restrict o);

#endif /* _OPTIONS_H_ */