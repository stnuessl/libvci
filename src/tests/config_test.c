#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <macros.h>
#include <config.h>

const char *keys[] = {
    "ShoppingList",
    "Year",
    "Month",
    "Day",
    "Name",
    "Age",
    "Distillery"
};

const char *sections[] = {
    NULL,
    "Whiskey",
    "Herb Liqueur",
    "Booze"
};

void *run(void *__restrict path)
{
    struct config *config;
    int sections_size, keys_size;
    const char **iterator1, **iterator2, *data;
    int err;
    
    config = config_new(path);
    if(!config)
        return NULL;
    
    err = config_parse(config);
    if(err < 0) {
        fprintf(stderr, "config parse: %d - %s\n", err, strerror(-err));
        return NULL;
    }
    
    sections_size = ARRAY_SIZE(sections);
    keys_size     = ARRAY_SIZE(keys);   
    
    for_each(sections, sections_size, iterator1) {
        for_each(keys, keys_size, iterator2) {
            do {
                data = config_value(config, *iterator1, *iterator2);
                if(data)
                    printf("[%s]: %s -> %s\n", *iterator1, *iterator2, data);
            } while(data);
        }
    }
    
    config_delete(config);
    
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t *threads;
    int i, err;

    if(argc < 2) {
        fprintf(stderr, 
                "Usage: %s <config01> <config02> <...>\n"
                "Example: %s config01.txt config02.txt\n",
                argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    
    threads = calloc(argc - 1, sizeof(*threads));
    assert(threads);
    
    for(i = 1; i < argc; ++i) {
        err = pthread_create(threads + i - 1, NULL, &run, argv[i]);
        assert(err == 0);
    }
    
    for(i = 1; i < argc; ++i)
        pthread_join(threads[i - 1], NULL);
    
    free(threads);
    
    return EXIT_SUCCESS;
}