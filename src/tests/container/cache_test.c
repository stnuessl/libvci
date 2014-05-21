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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <libvci/cache.h>
#include <libvci/compare.h>

unsigned int char_hash(const void *key)
{
    return (long)key;
}

void test_random(void)
{
#define N_ELEMENTS 1000000
    struct cache *cache;
    int fd, i, data, err;
    unsigned char key;
    unsigned int hit, miss;
    
    hit = 0;
    miss = 0;
    
    cache = cache_new(UCHAR_MAX / 2, &compare_int, &char_hash);
    assert(cache);
    
    fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    
    for(i = 0; i < UCHAR_MAX; i += 2)
        cache_insert(cache, (void *)(long)i, (void *)1);
    
    for(i = 0; i < N_ELEMENTS; ++i) {
        err = read(fd, &key, sizeof(key));
        assert(err == sizeof(key));
        
        data = (int)(long) cache_retrieve(cache, (void *)(long)key);
        if(data)
            hit += 1;
        else
            miss += 1;
    }
    
    fprintf(stdout, "Cache performance %d %%\n", 100 * hit / (hit + miss));
    
    close(fd);
    cache_delete(cache);
#undef N_ELEMENTS
}

void test_correctness(void)
{
#define CACHE_SIZE 10
#define N_ELEMENTS 20
    struct cache *cache;
    int i, data;
    
    cache = cache_new(CACHE_SIZE, &compare_int, &char_hash);
    assert(cache);

    for(i = 0; i < CACHE_SIZE; i++)
        cache_insert(cache, (void *)(long) i, (void *) 1);
    
    for(i = CACHE_SIZE; i < N_ELEMENTS; ++i)
        cache_insert(cache, (void *)(long) i, (void *) 1);
    
    for(i = 0; i < N_ELEMENTS; ++i) {
        data = (long) cache_retrieve(cache, (void *)(long) i);
        
        if(i < CACHE_SIZE)
            assert(!data);
        else
            assert(data);
    }

    cache_delete(cache);
#undef N_ELEMENTS
#undef CACHE_SIZE
}

int main(int argc, char *argv[])
{
    test_random();
    test_correctness();
    return EXIT_SUCCESS;
}