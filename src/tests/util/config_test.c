#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <libvci/macro.h>
#include <libvci/config.h>
#include <libvci/map.h>

#define PRINT_CONFIG(config,key)                                               \
    fprintf(stdout,                                                            \
    "%15s -> %20s\n",                                                          \
    key, config_value(config, key))

void *run(void *__restrict path)
{
    struct config *config;
    struct entry *e;
    const char *k;
    char *v;
    int err;
    
    config = config_new(path);
    assert(config);
    
    err = config_parse(config);
    assert(err == 0);
    
    PRINT_CONFIG(config, "Year");
    PRINT_CONFIG(config, "Month");
    PRINT_CONFIG(config, "Day");
    
    PRINT_CONFIG(config, "Name");
    PRINT_CONFIG(config, "Age");
    
    config_for_each(config, e) {
        k = entry_key(e);
        v = entry_data(e);
    
        fprintf(stdout, "%s -> %s\n", k, v);
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