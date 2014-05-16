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
#include <time.h>

#include <libvci/mempool.h>
#include <libvci/clock.h>
#include <libvci/macro.h>

#define LOOPS 1
#define POOL_CHUNKS 2000
#define USED_CHUNKS 2000
#define DATA_SIZE 64

struct a {
    char a[DATA_SIZE];
};

#define TYPE struct a

char mem[POOL_CHUNKS * sizeof(TYPE)];

void test_with_mempool(void)
{
    struct mempool *p;
    struct clock *c;
    TYPE *chunks[USED_CHUNKS];
    int i, j;
    
    p = mempool_new(mem, ARRAY_SIZE(mem), sizeof(TYPE));
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(p);
    assert(c);
    
    clock_start(c);

    for(i = 0; i < LOOPS; ++i) {

        for(j = 0; j < USED_CHUNKS; ++j) {
            chunks[j] = mempool_alloc_chunk(p);
            assert(chunks[j]);
            memset(chunks[j], 0xff, sizeof(TYPE));
        }

        for(j = 0; j < USED_CHUNKS; ++j)
            mempool_free_chunk(p, chunks[j]);
    }

    mempool_delete(p);
    
    printf("Test with Memory Pool: %lu us\n", clock_elapsed_us(c));
    
    clock_delete(c);
}

void test_without_mempool(void)
{
    TYPE *chunks[USED_CHUNKS];
    struct clock *c;
    int i, j;
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(c);
    
    clock_start(c);
    
    for(i = 0; i < LOOPS; ++i) {
        
        for(j = 0; j < USED_CHUNKS; ++j) {
            chunks[j] = malloc(sizeof(*chunks[j]));
            assert(chunks[j]);
            memset(chunks[j], 0xff, sizeof(TYPE));
        }

        
        for(j = 0; j < USED_CHUNKS; ++j)
            free(chunks[j]);
    }
    
    printf("Test without Memory Pool: %lu us\n", clock_elapsed_us(c));
    
    clock_delete(c);
}

void test_stability(void)
{
#define BUFFER_SIZE 1000
    struct mempool *p;
    char *tmp[BUFFER_SIZE];
    char mem_buffer[BUFFER_SIZE];
    int loops, i;
    
    p = mempool_new(mem_buffer, BUFFER_SIZE, sizeof(char));
    assert(p);
    
    loops = 1000;
    
    while(loops--) {
        for(i = 0; i < BUFFER_SIZE; ++i) {
            tmp[i] = mempool_alloc_chunk(p);
            assert(tmp[i]);
            
            memset(tmp[i], 0x55, sizeof(**tmp));
        }
        
        for(i = (BUFFER_SIZE / 2) - 1; i > -1; --i)
            mempool_free_chunk(p, tmp[i]); 
        
        for(i = 0; i < BUFFER_SIZE; ++i) {
            if(i >= BUFFER_SIZE / 2) {
                free(mempool_alloc_chunk(p));
            } else {
                tmp[i] = mempool_alloc_chunk(p);
                assert(tmp[i]);
                
                memset(tmp[i], 0x55, sizeof(**tmp));
            }
        }
        
        for(i = 0; i < BUFFER_SIZE; ++i)
            mempool_free_chunk(p, tmp[i]);
    }
    
    mempool_delete(p);
}

int main(int argc, char *argv[])
{
    
    test_stability();

    if(argc == 2) {
        if(strcmp(argv[1], "--no-mempool") == 0)
            test_without_mempool();
        else if(strcmp(argv[1], "--mempool") == 0)
            test_with_mempool();
    } else {
        test_without_mempool();
        test_with_mempool();
    }

    return EXIT_SUCCESS;
}