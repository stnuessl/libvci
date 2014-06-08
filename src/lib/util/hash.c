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

#include <limits.h>

#include "hash.h"

unsigned int hash_char(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_uchar(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_short(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_ushort(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_int(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_uint(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_long(const void *key)
{
    return hash_ulong(key);
}

unsigned int hash_ulong(const void *key)
{
    unsigned int hval;
    
    hval = 1; 
    
    hval += (unsigned long) key;
    hval += (hval << 10);
    hval ^= (hval >> 6);
    hval += ((unsigned long) key >> 32);
    hval &= 0x0fffffff;
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    return hval;
}

unsigned int hash_string(const void *key)
{
    const char *k;
    unsigned int hval;
    
    k = key;
    hval = 1;
    
    while(*k != '\0')
        hval += *k++;
    
    hval += (hval << 10);
    hval ^= (hval >> 6);
    hval &= 0x0fffffff;
    
    hval += (hval << 3);
    hval ^= (hval >> 11);
    hval += (hval << 15);
    
    return hval;
}