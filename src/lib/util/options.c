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

struct option {
    char *opt_str;
    char *s_opt;
    char *l_opt;
    const char *desc;
    
    enum value_type type;
    void *val;
};

static struct option *option_new(const char *str, 
                                 const char *d, 
                                 enum value_type type, 
                                 void *val)
{
    struct option *opt;
    char *p;
    
    opt = malloc(sizeof(*opt));
    if (!opt)
        return NULL;
    
    opt->opt_str = strdup(str);
    if (!opt->opt_str)
        goto cleanup1;
    
    opt->desc  = d;
    opt->type  = type;
    opt->val   = val;
    opt->l_opt = opt->opt_str;
    
    p = strchr(opt->opt_str, ',');
    if (!p) {
        opt->s_opt = NULL;
        return opt;
    }
    *p = '\0';
    
    opt->s_opt = p + 1;
    p = strchr(p + 1, ',');
    if (p) {
        /* Too many flags passed. */
        errno = EINVAL;
        goto cleanup2;
    }

    return opt;
    
cleanup2:
    free(opt->opt_str);
cleanup1:
    free(opt);
    return NULL;
}

static void option_delete(struct option *__restrict opt)
{
    free(opt->opt_str);
    free(opt);
}

static int option_map_insert(struct map *__restrict map, struct vector *opt_vec)
{
    struct option **data;
    int err;
    
    vector_for_each(opt_vec, data) {
        if ((*data)->s_opt) {
            err = map_insert(map, (*data)->s_opt, *data);
            if (err < 0)
                return err;
        }
        
        if ((*data)->l_opt) {
            err = map_insert(map, (*data)->l_opt, *data);
            if (err < 0)
                return err;
        }
    }
    
    return 0;
}

static int insert_dup(struct vector *__restrict vec, const char *__restrict s)
{
    char *dup = strdup(s);
    if (!dup)
        return -errno;
    
    return vector_insert_back(vec, dup);
}

int options_init(struct options *__restrict opts, const char *__restrict desc)
{
    int err;
    
    err = vector_init(&opts->opt_vec, 0);
    if (err < 0)
        goto out;
    
    vector_set_data_delete(&opts->opt_vec, (void (*)(void *)) &option_delete);
    
    err = vector_init(&opts->unknowns, 0);
    if (err < 0)
        goto cleanup1;
    
    err = vector_init(&opts->data, 0);
    if (err < 0)
        goto cleanup2;
    
    vector_set_data_delete(&opts->data, &free);
    
    opts->description   = desc;
    opts->last_err      = 0;
    opts->parse_err_msg = NULL;
    
    return 0;

cleanup2:
    vector_destroy(&opts->unknowns);
cleanup1:
    vector_destroy(&opts->opt_vec);
out:
    return err;
}

void options_destroy(struct options *__restrict opts)
{
    if (opts->parse_err_msg)
        free(opts->parse_err_msg);
    
    vector_destroy(&opts->data);
    vector_destroy(&opts->unknowns);
    vector_destroy(&opts->opt_vec);
}

int options_add(struct options *__restrict opts, 
                const char *str, 
                enum value_type type,
                void *__restrict val,
                const char *__restrict desc)
{
    struct option *opt;
    int err;
    
    if (!val) {
        opts->last_err = EINVAL;
        return -EINVAL;
    }
    
    opt = option_new(str, desc, type, val);
    if (!opt) {
        opts->last_err = -errno;
        return -errno;
    }
    
    err = vector_insert_back(&opts->opt_vec, opt);
    if (err < 0) {
        opts->last_err = -err;
        option_delete(opt);
        return err;
    }
    
    return 0;
}

static int option_parse(struct option *__restrict opt,
                        struct options *__restrict opts,
                        int *i,
                        const struct map *__restrict map)
{
    unsigned int j, size = vector_size(&opts->data);
    int err;

    /* get length of possible arguments */    
    
    for (j = *i + 1; j < size; ++j) {
        if (map_contains(map, *vector_at(&opts->data, j)))
            break;
    }
    
    switch (opt->type) {
        case OPTIONS_BOOL:
            *(bool *) opt->val = true;
            break;
        case OPTIONS_MUL_STRING:
            if (j == *i + 1) {
                asprintf(&opts->parse_err_msg, 
                            "option \"%s\" expects at least one argument.",
                            opt->l_opt);
                return -EINVAL;
            }
            
            /* add possible arguments to the vector */
            for (*i += 1; *i < j; ++(*i)) {
                char *arg = *vector_at(&opts->data, *i);
                
                err = vector_insert_back(opt->val, arg);
                if (err < 0)
                    return err;
            }
            
            *i -= 1;
            
            break;
        case OPTIONS_STRING:
            if (j == *i + 1) {
                asprintf(&opts->parse_err_msg, 
                         "option \"%s\" expects exactly one argument - "
                         "none provided.", opt->l_opt);
                return -EINVAL;
            }
            
            *i += 1;
            
            if (opt->val)
                *(char **)opt->val = *vector_at(&opts->data, *i);
            
            /* only one passed argument was expected */
            if (*i < j) {
                asprintf(&opts->parse_err_msg, 
                         "option \"%s\" expects exactly one argument - "
                         "%d provided.", opt->l_opt, j - *i - 1);
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

int options_parse(struct options *__restrict opts, 
                  const char **argv, 
                  int argc)
{
    struct map map;
    unsigned int size;
    int i, err;
    
    /* 
     * This function could be run multiple times and if one of the runs creates
     * an error message it has to be freed here to avoid a memory leak
     */
    if (opts->parse_err_msg) {
        free(opts->parse_err_msg);
        opts->parse_err_msg = NULL;
    }
    
    size = vector_size(&opts->opt_vec);
    
    err = map_init(&map, size, &compare_string, &hash_string);
    if (err < 0)
        return err;
    
    err = option_map_insert(&map, &opts->opt_vec);
    if (err < 0)
        goto cleanup1;
    
    /* 
     * Open up compact flags like -abcd into a vector { a b c d }
     * which enables to later parse the options without having a special
     * case for the compact flags
     */
    for (i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]);
        
        if (len > 1 && argv[i][0] == '-' && argv[i][1] != '-') {
            const char *p;
                
            for (p = argv[i] + 1; *p != '\0'; ++p) {
                char a[] = { *p, '\0' };

                err = insert_dup(&opts->data, a);
                if (err < 0) 
                    goto cleanup1;
            }
        } else if (len > 1 && argv[i][0] == '-' && argv[i][1] == '-') {
            err = insert_dup(&opts->data, argv[i] + 2);
            if (err < 0)
                return err;
        } else {
            err = insert_dup(&opts->data, argv[i]);
            if (err < 0)
                return err;
        }
    }
    
    /* parse all args */
    size = vector_size(&opts->data);
    
    for (i = 0; i < size; ++i) {
        char *arg = *vector_at(&opts->data, i);
        
        struct option *opt = map_retrieve(&map, arg);
        if (opt)
            err = option_parse(opt, opts, &i, &map);
        else
            err = vector_insert_back(&opts->unknowns, arg);
        
        if (err < 0)
            goto cleanup1;
    }
    
    map_destroy(&map);
    
    return 0;

cleanup1:
    map_destroy(&map);
    return err;
}

bool options_adding_ok(const struct options *__restrict opts)
{
    return opts->last_err == 0;
}

const char *options_adding_error(const struct options *__restrict opts)
{
    return strerr(opts->last_err);
}

const char *options_parse_error(const struct options *__restrict opts)
{
    return opts->parse_err_msg;
}

const struct vector *options_unknowns(const struct options *__restrict opts)
{
    return &opts->unknowns;
}
