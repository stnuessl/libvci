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


struct vector strings;
struct vector doubles;
struct vector ints;
char *string;
double double_val = 0.0;
int int_val = 0;
bool bool_val = false;
bool help = false;

struct program_option po[] = {
    { "strings",    "s", PO_STRING_VEC, &strings,    DESC_STRINGS    },
    { "doubles",    "d", PO_DOUBLE_VEC, &doubles,    DESC_DOUBLES    },
    { "ints",       "i", PO_INT_VEC,    &ints,       DESC_INTS       },
    { "string",     "S", PO_STRING,     &string,     DESC_STRING     },
    { "double-val", "D", PO_DOUBLE,     &double_val, DESC_DOUBLE     },
    { "int-val",    "I", PO_INT,        &int_val,    DESC_INT        },
    { "bool-val",   "B", PO_BOOL,       &bool_val,   DESC_BOOL       },
    { "help",       "" , PO_BOOL,       &help,       DESC_HELP       },
    { "",           "h", PO_BOOL,       &help,       DESC            },
};

char *argv_test[] = {
    "-s",               "file1",        "file2",        "file3",
    "--double-val",     "3.14159265",
    "-i",               "1",            "2",            "3",            "4",
    "-Bd",              "3.14159265",   "2.71828182846",
    "-I",               "631",
    "-S",               "Hello World!"
};

int argc_test = ARRAY_SIZE(argv_test);

struct options o;

static void vector_print(struct vector *__restrict vec)
{
    void **data, **end;
    struct vector *u = options_invalid_args(&o);
    
    fprintf(stdout, "[ ");
    
    end = vector_back(vec);
    
    vector_for_each(vec, data) {
        if (vec == &strings || vec == u)
            fprintf(stdout, "%s", * (char **) data);
        else if (vec == &doubles)
            fprintf(stdout, "%lf", **(double **) data);
        else if (vec == &ints)
            fprintf(stdout, "%i", **(int **) data);
        else
            return;
        
        if (data != end)
            fprintf(stdout, ", ");
    }
    
    fprintf(stdout, " ]\n");
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
    
    err = options_init(&o, po, ARRAY_SIZE(po));
    if (err < 0) {
        fprintf(stderr, "options_init() failed - %s\n", strerr(-err));
        exit(EXIT_FAILURE);
    }
    
    err = options_parse(&o, argv, argc, &err_msg);
    if (err < 0) {
        if (!err_msg) {
            fprintf(stderr, "options_init() failed: %s\n", strerr(-err));
        } else {
            fprintf(stderr, "options_init() failed: %s\n", err_msg);
            free(err_msg);
        }
        
        exit(EXIT_FAILURE);
    }
    
    clock_stop(&c);
    fprintf(stdout, "Elapsed time for parsing: %lu us\n", clock_elapsed_us(&c));
    
    vector_print(&strings);
    vector_print(&doubles);
    vector_print(&ints);
    
    fprintf(stdout, "double: %lf\n", double_val);
    fprintf(stdout, "int: %i\n", int_val);
    fprintf(stdout, "string: %s\n", string);
    fprintf(stdout, "bool: %s\n", (bool_val) ? "true" : "false");
    
    fprintf(stdout, "Invalid options: ");
    vector_print(options_invalid_args(&o));
    fprintf(stdout, "\n");
    
    if (help)
        options_help(&o, "options_test:", STDOUT_FILENO);
    
    options_clear(&o);
    assert(options_parse(&o, argv, argc, &err_msg) == 0);
    options_clear(&o);
    options_destroy(&o);
    clock_destroy(&c);

    return EXIT_SUCCESS;
}