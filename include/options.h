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

#include <map.h>
#include <vector.h>

enum value_type {
    OPTIONS_BOOL,
    OPTIONS_STRING,
    OPTIONS_MUL_STRING,
    OPTIONS_INT,
    OPTIONS_MUL_INT,
    OPTIONS_DOUBLE,
    OPTIONS_MUL_DOUBLE,
    OPTIONS_NONE,
};

struct options {
    const char *description;
    struct vector vec;
    struct map args;
    struct vector unknowns;
    int last_err;
};

int options_init(struct options *__restrict opts, const char *__restrict name);

void options_destroy(struct options *__restrict opts);

int options_add(struct options *__restrict opts, 
                const char *str, 
                enum value_type type,
                void *val,
                const char *desc);

int options_parse(const struct options *__restrict opts,
                  char *argv, 
                  int argc);

bool options_ok(const struct options *__restrict opts);

const char *options_last_error(const struct options *__restrict opts);

const struct map *options_args(const struct options *__restrict opts);

const struct vector *options_unknowns(const struct options *__restrict opts);

#endif /* _OPTIONS_H_ */