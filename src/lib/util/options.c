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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <vector.h>
#include <map.h>
#include <compare.h>
#include <hash.h>
#include <error.h>

#include <options.h>

struct option {
    char *opt_str;
    char *s_opt;
    char *l_opt;
    char *desc;
    
    enum value_type type;
    void *val;
};

static struct option *option_new(char *str, char *d, enum value_type type, void *val)
{
    struct option *opt;
    char *p, *q;
    
    opt = malloc(sizeof(*opt));
    if (!opt)
        return NULL;
    
    opt->opt_str = strdup(str);
    if (!opt->opt_str)
        goto cleanup1;
    
    opt->desc  = d;
    opt->type  = type;
    opt->val   = val;
    opt->l_opt = opt_str;
    
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
        err = map_insert(map, (*data)->s_opt, *data);
        if (err < 0)
            return err;
        
        err = map_insert(map, (*data)->l_opt, *data);
        if (err < 0)
            return err;
    }
    
    return 0;
}

int options_init(struct options *__restrict opts, const char *__restrict desc)
{
    int err;
    
    err = vector_init(&opts->vec, 0);
    if (err < 0)
        goto out;
    
    vector_set_data_delete(&opts->vec, (void (*)(void *)) &option_delete);
    
    err = map_init(&opts->args, 0, &compare_string, &hash_string);
    if (err < 0)
        goto cleanup1;
    
    err = vector_init(&opts->vec, 0);
    if (err < 0)
        goto cleanup2;
    
    opts->description = desc;
    opts->last_err = 0;
    
    return 0;
    
cleanup2:
    map_destroy(&opts->args);
cleanup1:
    vector_destroy(&opts->vec);
out:
    return err;
}

void options_destroy(struct options *__restrict opts)
{
    vector_destroy(&opts->unknowns);
    map_destroy(&opts->args);
    vector_destroy(&opts->vec);
    free(opts->name);
}

int options_add(struct options *__restrict opts, 
                const char *str, 
                enum value_type type,
                void *__restrict val,
                const char *__restrict desc)
{
    struct option *opt;
    int err;
    
    opt = option_new(str, desc, type, val);
    if (!opt)
        return -errno;
    
    err = vector_insert_back(&opts->vec, opt);
    if (err < 0) {
        option_delete(opt);
        return err;
    }
    
    return 0;
}

int options_parse(const struct options *__restrict opts, char *argv, int argc)
{
    struct map map;
    struct option *opt;
    char *arg;
    unsigned int size, len;
    int i, j, err;
    bool long_opt;
    
    size = vector_size(&opts->vec);
    
    err = map_init(&map, size, &compare_string, &hash_string);
    if (err < 0)
        return err;
    
    err = option_map_insert(&map, &opts->vec);
    if (err < 0)
        goto cleanup1;
    
    for (i = 0; i < argc; ++i) {
        len = strlen(argv[i]);
        
        if (len < 2) {
            vector_insert_back(&opts->unknowns, argv + i);
            continue;
        }
        
        /* 
         * Long option string with double '-' or short option with a 
         * single '-' ? 
         */
        if (argv[i][0] == '-' && argv[i][1] == '-') {
            opt = map_retrieve(&map, argv[i] + 2);
            if (!opt) {
                err = vector_insert_back(&opts->unknowns, argv +  i);
                if (err < 0)
                    goto cleanup1;
                
                continue;
            }
            
            /* get length of possible arguments */
            for (j = i + 1; !map_contains(&map, argv[j]) && j < argc; ++j)
                ;
            j -= 1;
            
            switch (opt.type) {
            case OPTIONS_BOOL:
            default:
                if (opt->val)
                    opt->val = (void *) true;
                
                continue;
            case OPTIONS_MUL_STRING:
                if (opt->val) {
                    if (j == i) {
                        /* no arguments passed */
                    }
                    
                    i += 1;
                    
                    while (i < j) {
                        err = vector_insert_back(opt->val, argv[i++]);
                        if (err < 0)
                            goto cleanup1;
                    }
                }
                
                continue;
            case OPTIONS_MUL_STRING:
                if (j == i) {
                    /* no argument passed */
                }
                
                i += 1;
                
                opt->val =  argv[i];
                
                /* only one passed argument was expected */
                while (i < j) {
                    err = vector_insert_back(&opts->unknowns, argv[i++]);
                    if (err < 0)
                        goto cleanup1;
                }
                
                continue;
            }
        } else if (argv[i][0] == '-') {
            argv = argv[i] + 1;
            long_opt = false;
        } else {
            vector_insert_back(&opts->unknowns, argv + i);
            continue;
        }
        

    }
    
    map_destroy(&map);
    
    return 0;
    
cleanup1:
    map_destroy(&map);
    return err;
}

bool options_ok(const struct options *__restrict opts)
{
    return opts->last_err == 0;
}

const char *options_last_error(const struct options *__restrict opts)
{
    return strerr(opts->last_err);
}

const struct map *options_args(const struct options *__restrict opts)
{
    return &opts->args;
}

const struct vector *options_unknowns(const struct options *__restrict opts)
{
    return &opts->unknowns;
}
