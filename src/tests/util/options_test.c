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
#include <stdbool.h>

#include <libvci/options.h>
#include <libvci/error.h>
#include <libvci/vector.h>
#include <libvci/macro.h>

#define DESC "Long description text goes here"

int main(int argc, char *argv[])
{
    struct options opts;
    struct vector adds, channels;
    struct vector *all[] = { &adds, &channels };
    int i, err;
    bool check = false;
    
    err = options_init(&opts, "option test");
    if (err < 0) {
        fprintf(stderr, "options_init() failed - %s\n", strerr(-err));
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < ARRAY_SIZE(all); ++i) {
        err = vector_init(all[i], 0);
        if (err < 0) {
            fprintf(stderr, "vector_init() failed - %s\n", strerr(-err));
            exit(EXIT_FAILURE);
        }
    }
    
    options_add(&opts, "add-bookmark,a", OPTIONS_MUL_STRING, &adds, DESC);
    options_add(&opts, "channels,c", OPTIONS_MUL_STRING, &channels, DESC);
    options_add(&opts, "check-bookmarks,b", OPTIONS_BOOL, &check,   DESC);
    
    if (!options_adding_ok(&opts)) {
        const char *msg = options_adding_error(&opts);
        fprintf(stderr, "options_adding_ok(): %s\n", msg);
        exit(EXIT_FAILURE);
    }
    
    err = options_parse(&opts, argv, argc);
    if (err < 0) {
        fprintf(stderr, "Parse error: %s\n", options_parse_error(&opts));
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < ARRAY_SIZE(all); ++i) {
        char **data;
        
        vector_for_each(all[i], data) {
            fprintf(stdout, "%s\n", *data);
        }
        
        fprintf(stdout, "\n");
    }
    
    if (check) {
        fprintf(stdout, "check enabled!\n");
    }
    
//     ("add-bookmark,a",    VAL_MUL(&args.adds),       DESC_ADD_B)
//     ("channels,C",        VAL_MUL(&args.channels),   DESC_CHANNELS)
//     ("check-bookmarks,b",                            DESC_CHECK_B)
//     ("descriptive,d",                                DESC_DESC)
//     ("featured,f",                                   DESC_FEATURED)
//     ("get-bookmarks",                                DESC_GET_F)
//     ("help,h",                                       DESC_HELP)
//     ("json,j",                                       DESC_JSON)
//     ("limit",             VAL(&args.limit),          DESC_LIMIT)
//     ("live",                                         DESC_LIVE)
//     ("remove-bookmark,r", VAL_MUL(&args.removes),    DESC_REMOVE_B)
//     ("search-channels,c", VAL_MUL(&args.s_channels), DESC_SEARCH_C)
//     ("search-games,g",    VAL_MUL(&args.s_games),    DESC_SEARCH_G)
//     ("search-streams,s",  VAL_MUL(&args.s_streams),  DESC_SEARCH_S)
//     ("streams,S",         VAL_MUL(&args.streams),    DESC_STREAMS)
//     ("top,t",                                        DESC_TOP)
//     ("verbose,v",                                    DESC_VERBOSE);
    
    for (i = 0; i < ARRAY_SIZE(all); ++i)
        vector_destroy(all[i]);
        
    options_destroy(&opts);
    
    return EXIT_SUCCESS;
}