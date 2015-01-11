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
#include <string.h>
#include <errno.h>

#include "vector.h"
#include "map.h"
#include "compare.h"
#include "hash.h"
#include "error.h"

#include "options.h"

static int option_parse(struct program_option *__restrict po,
                        int *i,
                        const struct map *__restrict map,
                        struct vector *__restrict args,
                        char **e_msg)
{
    unsigned int j, args_size = vector_size(args);
    int err;

    /* get length of possible arguments */
    
    for (j = *i + 1; j < args_size; ++j) {
        if (map_contains(map, *vector_at(args, j)))
            break;
    }
    
    switch (po->type) {
    case OPTIONS_BOOL:
        *(bool *) po->val = true;
        break;
    case OPTIONS_MUL_STRING:
        if (j == *i + 1) {
            if (e_msg) {
                asprintf(e_msg, "option \"%s\" expects at least one argument.",
                         po->cmd_flag_long);
            }
            
            return -EINVAL;
        }
        
        /* add possible arguments to the vector */
        for (*i += 1; *i < j; ++(*i)) {
            char *arg = strdup(*vector_at(args, *i));
            if (!arg)
                return -errno;
            
            err = vector_insert_back(po->val, arg);
            if (err < 0)
                return err;
        }
        
        *i -= 1;
        
        break;
    case OPTIONS_STRING:
        if (j == *i + 1) {
            if (e_msg) {
                asprintf(e_msg, "option \"%s\" expects exactly one argument - "
                              "none provided.", po->cmd_flag_long);
            }

            return -EINVAL;
        }
        
        *i += 1;

        *(char **) po->val = strdup(*vector_at(args, *i));
        if (!*(char **) po->val)
            return -errno;
        
        /* only one passed argument was expected */
        if (*i < j) {
            if (e_msg) {
                asprintf(e_msg, "option \"%s\" expects exactly one argument - "
                              "%d provided.", po->cmd_flag_long, j - *i - 1);
            }
            return -EINVAL;
        }
        break;
    case OPTIONS_MUL_INT:
        break;
    case OPTIONS_INT:
        break;
    case OPTIONS_MUL_DOUBLE:
        break;
    case OPTIONS_DOUBLE:
        break;
    }
    
    return 0;
}

static int options_init(struct program_option *__restrict po, 
                        unsigned int po_size)
{
    int i, err;
    
    for (i = 0; i < po_size; ++i) {
        /*
         * Only the vectorss get initialized; it is easy to set
         * default values for the other data types
         */
        switch (po[i].type) {
        case OPTIONS_MUL_STRING:
        case OPTIONS_MUL_DOUBLE:
        case OPTIONS_MUL_INT:
            err = vector_init(po[i].val, 0);
            if (err < 0)
                return err;
            
            vector_set_data_delete(po[i].val, &free);
            break;
        default:
            break;
        }
    }
    
    return 0;
}

int options_parse(struct program_option *__restrict po,
                  unsigned int po_size,
                  char ** const argv,
                  int argc,
                  char **err_msg)
{
    struct map map;
    struct vector args;
    unsigned int args_size;
    int i, err;
    
    if (err_msg && *err_msg)
        return -EINVAL;
    
    /* initialize options map */
    err = map_init(&map, po_size << 1, &compare_string, &hash_string);
    if (err < 0)
        return err;
    
    for (i = 0; i < po_size; ++i) {
        if (strlen(po[i].cmd_flag_long) > 0) {
            err = map_insert(&map, po[i].cmd_flag_long, po + i);
            if (err < 0)
                goto cleanup1;
        }
        
        if (strlen(po[i].cmd_flag_short) > 0) {
            err = map_insert(&map, po[i].cmd_flag_short, po + i);
            if (err < 0)
                goto cleanup1;
        }
    }
    
    /* 
     * Initialize 'args' which contains all elements of 'argv',
     * however compact flags like -abcd into a vector { a b c d }
     * which enables to later parse the options without having a special
     * case for the compact flags
     */
    
    err = vector_init(&args, argc);
    if (err < 0)
        goto cleanup1;
    
    vector_set_data_delete(&args, &free);
    
    for (i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]);
        
        if (len > 1 && argv[i][0] == '-' && argv[i][1] != '-') {
            const char *p;
            
            for (p = argv[i] + 1; *p != '\0'; ++p) {
                char a[] = { *p, '\0' };
                
                char *dup = strdup(a);
                if (!dup) {
                    err = -errno;
                    goto cleanup2;
                }
                
                err = vector_insert_back(&args, dup);
                if (err < 0)
                    goto cleanup2;
            }
        } else if (len > 1 && argv[i][0] == '-' && argv[i][1] == '-') {
            char *dup = strdup(argv[i] + 2);
            if (!dup) {
                err = -errno;
                goto cleanup2;
            }
            
            err = vector_insert_back(&args, dup);
            if (err < 0)
                goto cleanup2;
        } else {
            char *dup = strdup(argv[i]);
            if (!dup) {
                err = -errno;
                goto cleanup2;
            }
            
            err = vector_insert_back(&args, dup);
            if (err < 0)
                goto cleanup2;
        }
    }
    
    /* Initialize vectors of OPTIONS_MUL_* type */
    err = options_init(po, po_size);
    if (err < 0)
        goto cleanup2;
    
    args_size = vector_size(&args);
    
    for (i = 0; i < args_size; ++i) {
        char *arg = *vector_at(&args, i);
        
        struct program_option *o = map_retrieve(&map, arg);
        if (o)
            err = option_parse(o, &i, &map, &args, err_msg);
        
//         else
//             err = vector_insert_back(&opts->unknowns, arg);
        
        if (err < 0)
            goto cleanup3;
    }
    
    vector_destroy(&args);
    map_destroy(&map);
    
    return 0;

cleanup3:
    options_destroy(po, po_size);
cleanup2:
    vector_destroy(&args);
cleanup1:
    map_destroy(&map);
    return err;
}

void options_destroy(struct program_option *__restrict po,
                     unsigned int po_size)
{
    int i;
    
    for (i = 0; i < po_size; ++i) {
        switch (po[i].type) {
        case OPTIONS_MUL_STRING:
        case OPTIONS_MUL_DOUBLE:
        case OPTIONS_MUL_INT:
            vector_destroy(po[i].val);
            break;
        default:
            break;
        }
    }
}