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
#include <unistd.h>
#include <assert.h>

#include <libvci/log.h>


int main(int argc, char *argv[])
{
    struct log *l;
    
    /* open dummy */
    l = log_new("/tmp/log_test.txt", LOG_ALL);

    assert(l);
    
    log_set_level(l, LOG_INFO);
    log_debug(l, "module-1", "debug message\n");

    log_set_level(l, LOG_DEBUG);
    log_debug(l, "module-1", "debug message\n");
    
    log_info(l, "module-2", "info message\n");
    log_warning(l, NULL, "warning message\n");
    log_error(l, "module-1","error message\n");

    log_printf(l, LOG_WARNING, "module-3", "more warnings\n");
    
    fprintf(stdout, "log_print():\n");
    log_print(l, STDOUT_FILENO);
    
    log_delete(l);
    
    return 0;
}