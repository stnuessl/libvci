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


static int string_to_integer(const char *__restrict s, int *__restrict ret)
{
    char *end;
    
    errno = 0;
    *ret = (int) strtol(s, &end, 10);
    
    if (errno != 0)
        return -errno;
    
    if (*end != '\0')
        return -EINVAL;
    
    return 0;
}

static int string_to_double(const char *__restrict s, double *__restrict ret)
{
    char *end;
    
    errno = 0;
    *ret = strtod(s, &end);
    
    if (errno != 0)
        return -errno;
    
    if (*end != '\0')
        return -EINVAL;
    
    return 0;
}

static int assign_value(struct program_option *__restrict po, 
                        const char *__restrict val)
{
    double *d;
    char *c;
    int *i, err;
    
    switch (po->type) {
    case OPTIONS_DOUBLE:
        err = string_to_double(val, po->val);
        if (err < 0)
            return err;
        
        break;
    case OPTIONS_INT:
        err = string_to_integer(val, po->val);
        if (err < 0)
            return err;
        
        break;
    case OPTIONS_STRING:
        *(char **) po->val = strdup(val);
        if (!*(char **) po->val)
            return -errno;
        
        break;
    case OPTIONS_MUL_DOUBLE:
        d = malloc(sizeof(*d));
        if (!d)
            return -errno;
        
        err = string_to_double(val, d);
        if (err < 0) {
            free(d);
            return err;
        }
            
        err = vector_insert_back(po->val, d);
        if (err < 0) {
            free(d);
            return err;
        }
        break;
    case OPTIONS_MUL_INT:
        i = malloc(sizeof(*i));
        if (!i)
            return -errno;
        
        err = string_to_integer(val, i);
        if (err < 0) {
            free(i);
            return err;
        }
        
        err = vector_insert_back(po->val, i);
        if (err < 0) {
            free(i);
            return err;
        }
        
        break;
    case OPTIONS_MUL_STRING:
        c = strdup(val);
        if (!c)
            return -errno;
        
        err = vector_insert_back(po->val, c);
        if (err < 0) {
            free(c);
            return err;
        }
        break;
    default:
        return -EINVAL;
    }
    
    return 0;
}

static int option_parse(struct program_option *__restrict po,
                        int *i,
                        const struct map *__restrict map,
                        struct vector *__restrict args,
                        char **e_msg)
{
    unsigned int j, args_size = vector_size(args);
    int err;

    /* get length of possible arguments */
    
    if (po->type != OPTIONS_BOOL) {
        for (j = *i + 1; j < args_size; ++j) {
            if (map_contains(map, *vector_at(args, j)))
                break;
        }
    }
    
    /* 'j' describes the last valid index of a possible argument */
    j -= 1;
    
    switch (po->type) {
    case OPTIONS_BOOL:
        *(bool *) po->val = true;
        break;
    case OPTIONS_MUL_STRING:
    case OPTIONS_MUL_INT:
    case OPTIONS_MUL_DOUBLE:
        if (*i == j) {
            if (e_msg) {
                asprintf(e_msg, "option \"%s\" expects at least one argument.",
                         po->cmd_flag_long);
            }
            
            return -EINVAL;
        }
        
        /* add possible arguments to the vector */
        for (*i += 1; *i <= j; ++(*i)) {
            err = assign_value(po, *vector_at(args, *i));
            if (err < 0)
                return err;
        }
        
        *i -= 1;
        break;
    case OPTIONS_STRING:
    case OPTIONS_DOUBLE:
    case OPTIONS_INT:
        if (*i == j || j - 1 != *i) {
            if (e_msg) {
                asprintf(e_msg, "option \"%s\" expects exactly one argument - "
                              "%d provided.", po->cmd_flag_long, j - *i);
            }

            return -EINVAL;
        }
        
        *i += 1;
        
        err = assign_value(po, *vector_at(args, *i));
        if (err < 0)
            return err;
        
        break;
    default:
        return -EINVAL;
    }
    
    return 0;
}

static int options_init(struct program_option *__restrict po, 
                        unsigned int po_size)
{
    int i, err;
    
    for (i = 0; i < po_size; ++i) {
        /*
         * Only the vectors and strings get initialized; it is easy to set
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
        case OPTIONS_STRING:
            *(char **) po[i].val = NULL;
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
     * however compact flags like -abcd are split into a vector { a b c d }
     * which enables to later parse the options without having a special
     * case for the compact flags
     */
    
    err = vector_init(&args, argc);
    if (err < 0)
        goto cleanup1;
    
    for (i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]);
        
        if (len > 1 && argv[i][0] == '-' && argv[i][1] != '-') {
            const char *p;
            
            for (p = argv[i] + 1; *p != '\0'; ++p) {
                char a[] = { *p, '\0' };
                
                char *dup = strdupa(a);
                if (!dup) {
                    err = -errno;
                    goto cleanup2;
                }
                
                err = vector_insert_back(&args, dup);
                if (err < 0)
                    goto cleanup2;
            }
        } else if (len > 1 && argv[i][0] == '-' && argv[i][1] == '-') {
            err = vector_insert_back(&args, argv[i] + 2);
            if (err < 0)
                goto cleanup2;
        } else {
            err = vector_insert_back(&args, argv[i]);
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

void options_help(int fd,
                  const char *__restrict description, 
                  const struct program_option *__restrict po,
                  unsigned int po_size)
{
    unsigned int i;
    int max_s, max_l, max;
    
    if (description)
        dprintf(fd, "%s\n", description);
    
    for (i = 0, max_s = 0, max_l = 0; i < po_size; ++i) {
        int len_s = strlen(po[i].cmd_flag_short);
        int len_l = strlen(po[i].cmd_flag_long);
        
        if (len_s > max_s)
            max_s = len_s;
        
        if (len_l > max_l)
            max_l = len_l;
    }
    
    max = max_s + max_l;
    
    for (i = 0; i < po_size; ++i) {
        const char *p;
        const char *s = po[i].cmd_flag_short;
        const char *l = po[i].cmd_flag_long;
        const char *d = po[i].description;
        const char *arg = (po[i].type != OPTIONS_BOOL) ? "arg" : "   ";

        if (*s != '\0' && *l != '\0')
            dprintf(fd, "  -%-*s [ --%-*s ] %s", max_s, s, max_l, l, arg);
        else if (*s != '\0')
            dprintf(fd, "  -%-*s     %s   ", max, s, arg);
        else if (*l != '\0')
            dprintf(fd, "  --%-*s    %s   ", max, l, arg);
        else 
            continue;
        
        if (*d != '\0') {
            dprintf(fd, "   ");
            
            for (p = strchr(d, '\n'); p; d = p + 1, p = strchr(d, '\n')) {
                int diff = p - d;
                dprintf(fd, "%.*s\n   %-*s              ", diff, d, max, "   ");
            }
            
            dprintf(fd, "%s\n", d);
        }
    }
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
        case OPTIONS_STRING:
            free(*(char **) po[i].val);
            break;
        default:
            break;
        }
    }
}