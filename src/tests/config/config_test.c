#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <macro.h>
#include <config.h>
#include <map.h>

#define PRINT_CONFIG(config, section, key)                                     \
    fprintf(stdout,                                                            \
    "[%15s]: %15s -> %20s\n",                                                  \
    section ? section : "null", key, config_value(config, section, key))

void *run(void *__restrict path)
{
    struct config *config;
    int err;
    
    config = config_new(path);
    if(!config)
        return NULL;
    
    err = config_parse(config);
    if(err < 0)
        goto out;
    
    PRINT_CONFIG(config, NULL, "ShoppingList");
    PRINT_CONFIG(config, NULL, "Year");
    PRINT_CONFIG(config, NULL, "Month");
    PRINT_CONFIG(config, NULL, "Day");
    
    PRINT_CONFIG(config, "Whiskey", "Name");
    PRINT_CONFIG(config, "Whiskey", "Age");
    PRINT_CONFIG(config, "Whiskey", "Distillery");
    
    PRINT_CONFIG(config, "HerbLiqueur", "Name");
    PRINT_CONFIG(config, "Booze", "Name");
out:
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