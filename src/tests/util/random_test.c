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
#include <errno.h>
#include <time.h>
#include <assert.h>

#include <libvci/random.h>
#include <libvci/macro.h>
#include <libvci/map.h>

int int_compare(const void *a, const void *b)
{
    return a - b;
}

void test_seeding(unsigned int num)
{
    struct random *p1, *p2;
    unsigned int seed[] = { 1, 2, 3, 4 };
    int err;
    
    p1 = random_new();
    p2 = random_new();
    
    assert(p1);
    assert(p2);
    
    err = random_set_seed(p1, seed, ARRAY_SIZE(seed));
    assert(err == 0);
    
    err = random_set_seed(p2, seed, ARRAY_SIZE(seed));
    assert(err == 0);
    
    while(num--)
        assert(random_uint(p1) == random_uint(p2));
    
    random_delete(p1);
    random_delete(p2);
}

void test_randomness(void)
{
    struct random *rand;
    unsigned int n, i, num;
    
    rand = random_new();
    assert(rand);
    
    num = 100;
    
    for(i = 0; i < num; ++i) {
        n = random_uint(rand);
        
        fprintf(stdout, "%-10u\t", n);
        
        if((i % 3) == 0)
            fprintf(stdout, "\n");
    }
    
    fprintf(stdout, "\n");
    
    random_delete(rand);
}

void test_duplicates(unsigned int num)
{
    struct map *m;
    struct random *random;
    unsigned int i, n, dups1, dups2;;
    int err;
    
    m = map_new(0, &int_compare, &hash_u32);
    random = random_new();
    
    assert(m);
    assert(random);
    
    dups1 = 0;
    
    for(i = 0; i < num; ++i) {
        n = random_uint(random);
        
        if(map_contains(m, (void *)(long) n)) {
            dups1 += 1;
        } else {
            err = map_insert(m, (void *)(long) n, (void *)(long) n);
            if(err < 0) {
                fprintf(stderr, "Warning: "
                        "Map unable to insert %u in %uth iteration: %s\n",
                        n, i, strerror(-err));
            }
        }
    }
    
    random_delete(random);
    map_clear(m);
    
    srand(time(NULL));
    
    dups2 = 0;
    
    for(i = 0; i < num; ++i) {
        n = rand();
        
        if(map_contains(m, (void *)(long) n)) {
            dups2 += 1;
        } else {
            err = map_insert(m, (void *)(long) n, (void *)(long) n);
            if(err < 0) {
                fprintf(stderr, "Warning: "
                "Map unable to insert %u in %uth iteration: %s\n",
                n, i, strerror(-err));
            }
        }
    }
    
    map_delete(m);
    
    fprintf(stdout,
            "random: After %u iterations there are %u (%lf %%) duplicates\n"
            "rand(): After %u iterations there are %u (%lf %%) duplicates\n", 
            num, dups1, (double) dups1 / num, num, dups2, (double) dups2 / num);
}

void test_range(unsigned int num)
{
    struct random *rand;
    unsigned int i, n;
    
    rand = random_new();
    assert(rand);
    
    for(i = 0; i < num; ++i) {
        n = random_uint_range(rand, 0, 100);
        assert(n >= 0 && n <= 100);
        
        n = random_uint_range(rand, 1000, 10000);
        assert(n >= 1000 && n <= 10000);
        
        n = random_uint_range(rand, 99999, 123456);
        assert(n >= 99999 && n <= 123456);
        
        n = random_uint_range(rand, (unsigned int) 1e6 + 1, (unsigned int) 1e8);
        assert(n >= (unsigned int) 1e6 + 1 && n <= (unsigned int) 1e8);
        
        n = random_uint_range(rand, 0, 0);
        assert(n == 0);
        
        n = random_uint_range(rand, 100, 100);
        assert(n == 100);
    }
    
    random_delete(rand);
}

int main(int argc, char *argv[])
{
    unsigned int num;
    
    if(argc == 2)
        num = atoi(argv[1]);
    else
        num = 0;
    
    test_randomness();
    
    if(num) {
        test_seeding(num);
        test_duplicates(num);
        test_range(num);
    }
    
    fprintf(stdout, "Tests finished\n");
    
    return EXIT_SUCCESS;
}