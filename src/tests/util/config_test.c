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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <libvci/macro.h>
#include <libvci/config.h>
#include <libvci/map.h>

#define PRINT_CONFIG(config,key)                                               \
    fprintf(stdout,                                                            \
    "%15s -> %20s\n",                                                          \
    key, config_value(config, key))
    
static void print_key_value_pair(const char *key, const char *val, void *arg)
{
    fprintf(stdout, " %6s -> %s\n", key, val);
}

static struct config_handle handles[] = {
    { &print_key_value_pair,    "Year",      NULL },
    { &print_key_value_pair,    "Month",     NULL },
    { &print_key_value_pair,    "Day",       NULL },
    { &print_key_value_pair,    "Name",      NULL },
    { &print_key_value_pair,    "Age",       NULL }
};

static char conf_txt[] = {
    "#\n"
    "# Example config file which can by accessed\n"
    "# via the config API calls of the vci library.\n"
    "# Keys and values must beginn with a letter or a digit.\n"
    "#\n"
    "\n"
    "#\n"
    "# the parser ignores lines beginning with a '#'\n"
    "#\n"
    ";\n"
    "; same goes for lines with a ';'\n"
    ";\n"
    "\n"
    "Year   = 2013\n"
    "Month  = September\n"
    "Day    = Wednesday\n"
    "\n"
    "Name = Doge\n"
    "Age  = 1\n"
};

static void text_init(int fd, void *arg)
{
    dprintf(fd, "%s\n", (char *) arg);
}

int main(int argc, char *argv[])
{
    struct config *config;
    struct entry *e;
    const char *k;
    char *v;
    int i, err;
    
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <path where to dump config>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    config = config_new(argv[1], &text_init, conf_txt);
    assert(config);
    
    for(i = 0; i < ARRAY_SIZE(handles); ++i) {
        err = config_insert_handle(config, handles + i);
        assert(err == 0);
    }
    
    fprintf(stdout, "starting parsing...\n");
    /* multiple parser runs shouldn't have an effect on ressource allocations */
    err = config_parse(config);
    assert(err == 0);
    
    fprintf(stdout, "finished parsing...\n");
    
    for(i = 0; i < ARRAY_SIZE(handles); ++i)
        assert(config_take_handle(config, handles + i) == (handles + i));
    
    err = config_parse(config);
    assert(err == 0);
    
    PRINT_CONFIG(config, "Year");
    PRINT_CONFIG(config, "Month");
    PRINT_CONFIG(config, "Day");
    
    PRINT_CONFIG(config, "Name");
    PRINT_CONFIG(config, "Age");
    
    config_for_each(config, e) {
        k = entry_key(e);
        v = entry_data(e);
        
        fprintf(stdout, "%s -> %s\n", k, v);
    }
    
    config_delete(config);
    
    return EXIT_SUCCESS;
}