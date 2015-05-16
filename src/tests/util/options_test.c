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
#include <stdbool.h>
#include <assert.h>

#include <libvci/options.h>
#include <libvci/error.h>
#include <libvci/vector.h>
#include <libvci/macro.h>
#include <libvci/clock.h>

#define DESC "very\nlong\ndescription"
#define DESC_STRINGS "Pass one or more strings."
#define DESC_DOUBLES "Pass one or more doubles."
#define DESC_INTS    "Pass one or more ints."
#define DESC_STRING  "Pass a string."
#define DESC_DOUBLE  "Pass a double."
#define DESC_INT     "Pass a int."
#define DESC_BOOL    "Activates this flag."
#define DESC_HELP    "Print this help message."



struct program_option po[] = {
    PROGRAM_OPTION_INIT("--opt1", "-1", 2),
    PROGRAM_OPTION_INIT("--opt2", "-2", 0),
    PROGRAM_OPTION_INIT("--opt3", "-3", 1),
    PROGRAM_OPTION_INIT("--opt4", "-4", -1),
};

/* Make an easy mapping to the options */
struct program_option *opt1 = po + 0;
struct program_option *opt2 = po + 1;
struct program_option *opt3 = po + 2;
struct program_option *opt4 = po + 3;


char *argv_test[] = {
    "options_test",
    "-1234",   "arg11", "arg12", "arg31", "arg41", "arg42", "arg43", "arg44",
};

int argc_test = ARRAY_SIZE(argv_test);


void print_opt(struct program_option *__restrict po)
{
    
    if (po->argc <= 0) {
        const char *passed = (po->passed) ? "passed with no arguments" : "";
        
        fprintf(stdout, "%s: { %s }\n", po->cmd_flag_long, passed);
    } else {
        fprintf(stdout, "%s: { ", po->cmd_flag_long);
        
        for (int i = 0; i < po->argc - 1; ++i) {
            fprintf(stdout, "%s, ", po->argv[i]);
        }
        
        fprintf(stdout, "%s }\n", po->argv[po->argc - 1]);
    }
}

int main(int argc, char *argv[])
{
    struct clock c;
    char *err_msg = NULL;
    int err;
    
    if (argc <= 1) {
        argv = argv_test;
        argc = argc_test;
    }
    
    assert(clock_init(&c, CLOCK_PROCESS_CPUTIME_ID) == 0 && "clock_init");
    
    clock_start(&c);
        
    err = options_parse(po, ARRAY_SIZE(po), argv + 1, argc - 1, &err_msg);
    if (err < 0) {
        if (!err_msg) {
            fprintf(stderr, "options_parse() failed: %s\n", strerr(-err));
        } else {
            fprintf(stderr, "options_parse() failed: %s\n", err_msg);
            free(err_msg);
        }
        
        exit(EXIT_FAILURE);
    }
    
    clock_stop(&c);
    fprintf(stdout, "Elapsed time for parsing: %lu us\n", clock_elapsed_us(&c));
    
    print_opt(opt1);
    print_opt(opt2);
    print_opt(opt3);
    print_opt(opt4);
   
    clock_destroy(&c);

    return EXIT_SUCCESS;
}