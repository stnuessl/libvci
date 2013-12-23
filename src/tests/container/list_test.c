#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <list.h>
#include <macros.h>

struct data {
    int a;
    struct link link;
};

int main(int argc, char *argv[])
{
    struct link list, *tmp;
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
    
    list_for_each(&list, tmp) {
        d = container_of(tmp, struct data, link);
        
        printf("value: %d\n", d->a);
    }
   
    return EXIT_SUCCESS;
}