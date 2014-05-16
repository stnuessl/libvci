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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdbool.h>

#include "map.h"
#include "vector.h"
#include "list.h"

struct config {
    struct map map;
    char *path;
    char *mem;
};

struct config *config_new(const char *__restrict path);

void config_delete(struct config *__restrict config);

int config_init(struct config *__restrict config, const char *__restrict path);

void config_destroy(struct config *__restrict config);

int config_parse(struct config *__restrict config);

const char *config_value(struct config *__restrict config, 
                         const char *__restrict key);

int config_add_value(struct config *__restrict config, 
                     const char *key,
                     char *value);

int config_set_path(struct config *__restrict config, 
                    const char *__restrict path);

const char *config_path(struct config *__restrict config);

#define config_for_each(config, entry)                                         \
    map_for_each(&(config)->map, (entry))

#endif /* _CONFIG_H_ */  