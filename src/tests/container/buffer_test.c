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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libvci/buffer.h>
#include <libvci/macro.h>

int main(int argc, char *argv[])
{
    struct buffer buf;
    int err;
    
    err = buffer_init(&buf, 1);
    assert(err == 0);
    
    while(argc--) {
        err = buffer_prepare_write(&buf, strlen(argv[argc]) + sizeof(char));
        assert(err == 0);
        
        buffer_write(&buf, argv[argc], strlen(argv[argc]));
        buffer_write_char(&buf, '\n');
    }
    
    //buffer_remove_unaccessed(&buf);
    
    while(buffer_bytes_accessible(&buf) > 0)
        fprintf(stdout, "%c", buffer_read_char(&buf));
    
    buffer_clear_accessed(&buf);
    
    while(buffer_bytes_accessible(&buf) > 0)
        fprintf(stdout, "%c", buffer_read_char(&buf));
    
    buffer_destroy(&buf);
        
    return EXIT_SUCCESS;
}