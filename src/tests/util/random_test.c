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
    }
    
    fprintf(stdout, "Tests finished\n");
    
    return EXIT_SUCCESS;
}