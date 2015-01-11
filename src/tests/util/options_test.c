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

#define DESC_ADD_B    "Add a new bookmark."
#define DESC_CHANNELS "Retrieve information about a channel."
#define DESC_CHECK_B  "Check which bookmarks are streaming."
#define DESC_DESC     "Print descriptive line headers, if applicable." 
#define DESC_FEATURED "Query featured streams."
#define DESC_GET_F    "Show all specified bookmarks."
#define DESC_HELP     "Print this help message."
#define DESC_JSON     "Pretty print the json strings sent from the server."
#define DESC_LIMIT    "Set the number of returned results."
#define DESC_LIVE     "If searching for games: list only games that are live."
#define DESC_REMOVE_B "Remove a bookmark."
#define DESC_SEARCH_C "Search for channels."
#define DESC_SEARCH_G "Search for games."
#define DESC_SEARCH_S "Search for streams."
#define DESC_STREAMS  "Retrieve information about a steam. Stream must be live."
#define DESC_TOP      "Get a list of the currently top played games."
#define DESC_VERBOSE  "Retrieve more information about queried items."

struct vector adds;
struct vector channels;
struct vector streams;
struct vector removes;
struct vector s_streams;
bool check_bookmarks = false;
bool descriptive = false;
bool featured = false;
bool json = false;
int limit = 10;

struct program_option po[] = {
    { "add-bookmarks",   "a", OPTIONS_MUL_STRING, &adds,            DESC_ADD_B    },
    { "channels"     ,   "c", OPTIONS_MUL_STRING, &channels,        DESC_CHANNELS },
    { "check-bookmarks", "b", OPTIONS_BOOL,       &check_bookmarks, DESC_CHECK_B  },
    { "descriptive",     "d", OPTIONS_BOOL,       &descriptive,     DESC_DESC     },
    { "featured",        "f", OPTIONS_BOOL,       &featured,        DESC_FEATURED },
    { "json",            "j", OPTIONS_BOOL,       &json,            DESC_JSON     },
    { "limit",           "l", OPTIONS_INT,        &limit,           DESC_LIMIT    },
    { "streams",         "s", OPTIONS_MUL_STRING, &streams,         DESC_STREAMS  },
    { "remove-bookmark", "r", OPTIONS_MUL_STRING, &removes,         DESC_REMOVE_B },
    { "search-streams",  "s", OPTIONS_MUL_STRING, &s_streams,       DESC_REMOVE_B },
};

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
//     ("verbose,v",  

int main(int argc, char *argv[])
{
    struct vector *all[] = { &adds, &channels };
    char *err_msg = NULL;
    int i, err;
    
    err = options_parse(po, ARRAY_SIZE(po), argv, argc, &err_msg);
    if (err < 0) {
        fprintf(stderr, "options_parse() failed: %s\n", err_msg);
        free(err_msg);
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < ARRAY_SIZE(all); ++i) {
        char **data;
        
        vector_for_each(all[i], data) {
            fprintf(stdout, "%s\n", *data);
        }
        
        fprintf(stdout, "\n");
    }
    
    if (check_bookmarks) {
        fprintf(stdout, "check enabled!\n");
    }
    
    options_destroy(po, ARRAY_SIZE(po));

    return EXIT_SUCCESS;
}