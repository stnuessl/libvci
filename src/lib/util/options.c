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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "vector.h"
#include "map.h"
#include "compare.h"
#include "hash.h"
#include "error.h"

#include "options.h"

static int error_unknown_option(const char *__restrict o, char **msg)
{
    if (msg)
        asprintf(msg, "unknown option \"%s\"", o);
    
    return -EINVAL;
}

static int error_missing_arguments(const char *__restrict o, int n, char **msg)
{
    if (msg)
        asprintf(msg, "option \"%s\" expects %d more argument(s)", o, n);
    
    return -ENODATA;
}

static int error_dupped_option(const char *__restrict o, char **msg)
{
    if (msg)
        asprintf(msg, "option \"%s\" got passed twice", o);
    
    return -ENOTSUP;
}

static int handle_argument(const char *__restrict arg, 
                           struct map *__restrict map,
                           int *__restrict i,
                           char ** const argv,
                           int argc,
                           char **err_msg)
{
    struct program_option *po = map_retrieve(map, arg);
    int j = *i + 1;
    
    if (!po)
        return error_unknown_option(arg, err_msg);
    
    if (po->passed)
        return error_dupped_option(arg, err_msg);
    
    po->passed = true;
    
    if (po->accepts < 0) {
        while (j < argc && argv[j][0] != '-')
            ++j;
        
        --j;
        
        po->argc = j - *i;
        
        if (po->argc == 0)
            return error_missing_arguments(arg, 1, err_msg);
        
        po->argv = (const char **) argv + *i + 1;
        
        *i = j;
    } else if (po->accepts > 0) {
        while(j < argc && argv[j][0] != '-' && j - *i - 1 < po->accepts)
            ++j;
        
        --j;
        
        po->argc = j - *i;
        
        if (po->argc != po->accepts) {
            int missing = po->accepts - po->argc;
            
            return error_missing_arguments(arg, missing, err_msg);
        }
        
        po->argv = (const char **) argv + *i + 1;
        
        *i = j;
    } else {
        po->argv = NULL;
        po->argc = 0;
    }
    
    return 0;
}

int options_parse(struct program_option *__restrict po, 
                  unsigned int size,
                  char ** const argv, 
                  int argc, 
                  char **err_msg)
{
    const struct map_config map_conf = {
        .size           = size << 1,
        .lower_bound    = MAP_DEFAULT_LOWER_BOUND,
        .upper_bound    = MAP_DEFAULT_UPPER_BOUND,
        .static_size    = false,
        .key_compare    = &compare_string,
        .key_hash       = &hash_string,
        .data_delete    = NULL,
    };
    struct map map;
    int err;
    
    /* initialize options map */
    err = map_init(&map, &map_conf);
    if (err < 0)
        return err;
    
    for (unsigned int i = 0; i < size; ++i) {
        po[i].passed = false;
        
        if (*po[i].cmd_flag_long != '\0') {
            err = map_insert(&map, po[i].cmd_flag_long, po + i);
            if (err < 0)
                goto cleanup1;
        }
        
        if (*po[i].cmd_flag_short != '\0') {
            err = map_insert(&map, po[i].cmd_flag_short, po + i);
            if (err < 0)
                goto cleanup1;
        }
    }

    
    for (int i = 0; i < argc; ++i) {
        if (strlen(argv[i]) < 2 || argv[i][0] != '-') {
            if (err_msg)
                asprintf(err_msg, "expected option, got \"%s\"", argv[i]);
            
            err = -EINVAL;
            goto cleanup1;
        }

        if (argv[i][0] == '-' && argv[i][1] != '-') {
            for (char *p = argv[i] + 1; *p != '\0'; ++p) {
                const char arg[] = { '-', *p, '\0' };
                
                err = handle_argument(arg, &map, &i, argv, argc, err_msg);
                if (err < 0)
                    goto cleanup1;
            }
        } else {
            err = handle_argument(argv[i], &map, &i, argv, argc, err_msg);
            if (err < 0)
                goto cleanup1;
        }
    }

cleanup1:
    map_destroy(&map);
    return err;
}
