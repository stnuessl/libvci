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

#include <string.h>

int compare_char(const void *c1, const void *c2)
{
    char a, b;
    
    a = (long) c1;
    b = (long) c2;
    
    return (a > b) - (a < b);
}

int compare_uchar(const void *c1, const void *c2)
{
    unsigned char a, b;
    
    a = (long) c1;
    b = (long) c2;
    
    return (a > b) - (a < b);
}

int compare_short(const void *s1, const void *s2)
{
    short a, b;
    
    a = (long) s1;
    b = (long) s2;
    
    return (a > b) - (a < b);
}

int compare_ushort(const void *s1, const void *s2)
{
    unsigned short a, b;
    
    a = (long) s1;
    b = (long) s2;
    
    return (a > b) - (a < b);
}

int compare_int(const void *i1, const void *i2)
{
    int a, b;
    
    a = (long) i1;
    b = (long) i2;
    
    return (a > b) - (a < b);
}

int compare_uint(const void *i1, const void *i2)
{
    unsigned int a, b;
    
    a = (long) i1;
    b = (long) i2;
    
    return (a > b) - (a < b);
}

int compare_long(const void *l1, const void *l2)
{
    long a, b;
    
    a = (long) l1;
    b = (long) l2;
    
    return (a > b) - (a < b);
}

int compare_ulong(const void *l1, const void *l2)
{
    unsigned long a, b;
    
    a = (unsigned long) l1;
    b = (unsigned long) l2;
    
    return (a > b) - (a < b);
}

int compare_string(const void *s1, const void *s2)
{
    return strcmp(s1, s2);
}
