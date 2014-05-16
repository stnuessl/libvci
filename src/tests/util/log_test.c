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

#include <assert.h>

#include <libvci/log.h>


int main(int argc, char *argv[])
{
    struct log *l1, *l2;
    uint8_t flags;
    
    flags = LOG_DATE | 
            LOG_PID | 
            LOG_NAME | 
            LOG_LEVEL;
                
    /* open dummy */
    l1 = log_new("/dev/null", "MyComponent", flags);
    l2 = log_new("/dev/null", "MyOtherComponent", LOG_ALL);

    assert(l1);
    assert(l2);
    
    log_set_severity_cap(l1, LOG_INFO);
    log_set_severity_cap(l2, LOG_INFO);
    
    /* set file now */
    log_set_file(l1, stdout);
    log_set_file(l2, stdout);
    
    log_error(l1, "error message\n");
    log_critical(l1, "critical message\n");
    log_warning(l1, "warning message\n");
    log_message(l2, "general message\n");
    log_debug(l2, "debug message\n");
    log_info(l2, "info message\n");
    
    log_delete(l1);
    log_delete(l2);
    
    return 0;
}