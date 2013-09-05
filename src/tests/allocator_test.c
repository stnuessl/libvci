#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <stack.h>
#include <mempool.h>
#include <clock.h>
#include <item_allocator.h>

#define CHUNKS_USED 5000
#define MEMPOOL_NUM_CHUNKS 10000

#define DATA_SIZE 256
#define KEY_SIZE 64

struct data {
    char a[DATA_SIZE];
};

struct key {
    char a[KEY_SIZE];
};

struct clock *clk;
struct stack *stack;
struct data *data;
struct key *key;
struct mempool *data_pool, *key_pool;

static void _key_populate(struct key *key)
{
    int i;
    static int a = 0;
    
    for(i = 0; i < KEY_SIZE; ++i)
        key->a[i] = a++;
}

static void _data_populate(struct data *data)
{
    data->a[0] = 1;
}

static void _key_delete(void *key)
{
    mempool_free_chunk(key_pool, key);
}

static void _data_delete(void *data)
{
    mempool_free_chunk(data_pool, data);
}


void test_with_mempools(void)
{
    int i, err;
    
    stack     = stack_new();
    clk       = clock_new(CLOCK_MONOTONIC);
    data_pool = mempool_new(MEMPOOL_NUM_CHUNKS, sizeof(*data));
    key_pool  = mempool_new(MEMPOOL_NUM_CHUNKS, sizeof(*key));
    
    assert(stack);
    assert(clk);
    assert(data_pool);
    assert(key_pool);
    
    stack_set_data_delete(stack, &_data_delete);
    stack_set_key_delete(stack, &_key_delete);

    clock_start(clk);
    
    for(i = 0; i < CHUNKS_USED; ++i) {
        data = mempool_alloc_chunk(data_pool);
        key  = mempool_alloc_chunk(key_pool);
        
        assert(data);
        assert(key);
        
        _data_populate(data);
        _key_populate(key);
        
        err = stack_push(stack, data, key);
        assert(err == 0);
    }
    
    clock_stop(clk);
    
    fprintf(stdout, "-> With mempools: %lu us\n", clock_elapsed_us(clk));
    
    stack_delete(stack);
    clock_delete(clk);
    mempool_delete(data_pool);
    mempool_delete(key_pool);
}

void test_without_mempools(void)
{
    int i, err;
    
    stack = stack_new();
    clk   = clock_new(CLOCK_MONOTONIC);
    
    assert(stack);
    assert(clk);
    
    stack_set_data_delete(stack, &free);
    stack_set_key_delete(stack, &free);
    
    clock_start(clk);
    
    for(i = 0; i < CHUNKS_USED; ++i) {
        data = malloc(sizeof(*data));
        key  = malloc(sizeof(*key));

        assert(data);
        assert(key);
        
        _data_populate(data);
        _key_populate(key);
        
        err = stack_push(stack, data, key);
        assert(err == 0);
    }
    
    clock_stop(clk);
    
    fprintf(stdout, "-> Without mempools: %lu us\n", clock_elapsed_us(clk));
    
    stack_delete(stack);
    clock_delete(clk);
}

int main(int argc, char *argv[])
{
    fprintf(stdout, "Item allocator initialized\n");
    assert(item_allocator_init(MEMPOOL_NUM_CHUNKS) == 0);
    test_with_mempools();
    item_allocator_destroy();
    assert(item_allocator_init(MEMPOOL_NUM_CHUNKS) == 0);
    test_without_mempools();
    
    fprintf(stdout, "Item allocator uninitialized\n");
    item_allocator_destroy();
    test_with_mempools();
    item_allocator_destroy();
    test_without_mempools();
    fprintf(stdout, "\n");

    return EXIT_SUCCESS;
}