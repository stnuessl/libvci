#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <queue.h>
#include <clock.h>
#include <macro.h>



struct data {
    unsigned int data;
    struct link link;
};

void init_data(struct data *data, unsigned int size)
{
    while(size--)
        data[size].data = size;
}

void test_functionality(void)
{
#define QUEUE_SIZE 10
    struct queue *q;
    struct data data[QUEUE_SIZE], *tmp;
    int i;
    
    q = queue_new();
    assert(q);
    
    init_data(data, ARRAY_SIZE(data));
    
    for(i = 0; i < ARRAY_SIZE(data); ++i)
        queue_insert(q, &data[i].link);
    
    while(!queue_empty(q)) {
        tmp = container_of(queue_take(q), struct data, link);
        fprintf(stdout, "%u ", tmp->data);
    }
    
    fprintf(stdout, "\n");
    
    queue_delete(q, NULL);
#undef QUEUE_SIZE
}



void test_performance(void)
{
#define QUEUE_SIZE 1000000
    static struct data data[QUEUE_SIZE];
    struct clock *c;
    struct queue *q;

    int i;

    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    q = queue_new();
    assert(c);
    assert(q);
    
    clock_start(c);
    
    for(i = 0; i < QUEUE_SIZE; ++i)
        queue_insert(q, &data[i].link);
    
    while(!queue_empty(q))
        assert(container_of(queue_take(q), struct data, link));
    
    fprintf(stdout, "Time elapsed for %u elements: %lu us\n",
            QUEUE_SIZE, clock_elapsed_us(c));
    
    clock_delete(c);
    queue_delete(q, NULL);
#undef QUEUE_SIZE
}

int main(int argc, char *argv[])
{
    test_functionality();
    test_performance();
    
    return EXIT_SUCCESS;
}