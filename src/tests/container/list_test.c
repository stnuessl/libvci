#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <list.h>
#include <clock.h>
#include <macros.h>

struct data {
    int a;
    struct link link;
};

void data_delete_from_link(struct link *link)
{
    free(container_of(link, struct data, link));
}

void list_test_usage(void)
{
    struct link list, *tmp;
    struct data *data;
    struct clock *c;
    int i, num;
    
    num = 10000000;
    
    list_init(&list);
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    assert(c);
    
    for(i = 0; i < num; ++i) {
        data = malloc(sizeof(*data));
        
        data->a = i;
        list_insert(&list, &data->link);
    }
    
    clock_start(c);
    
    list_for_each(&list, tmp) {
        data = container_of(tmp, struct data, link);
        
        if(data->a == 0)
            fprintf(stdout,
                    "Accessed last element (of %d) in list within %lu ms.\n",
                    num, clock_elapsed_ms(c));
    }
    
    clock_delete(c);
    list_destroy(&list, &data_delete_from_link);
}

int main(int argc, char *argv[])
{
    struct link list, *link, *tmp;
    struct data a = {
        .a = 10
    }, b = {
        .a = 11 
    }, c = {
        .a = 12
    }, *d;
    
    list_init(&list);
    
    list_insert(&list, &a.link);
    list_insert(&list, &b.link);
    list_insert(&list, &c.link);
    
    printf("List iteration forward.\n");
    
    list_for_each(&list, link) {
        d = container_of(link, struct data, link);
        
        printf("value: %d\n", d->a);
    }
    
    printf("List iteration backward.\n");
    
    list_for_each_reverse_safe(&list, link, tmp) {
        d = container_of(link, struct data, link);
        
        printf("value: %d\n", d->a);
        link = NULL;
    }
    
    list_test_usage();
   
    return EXIT_SUCCESS;
}