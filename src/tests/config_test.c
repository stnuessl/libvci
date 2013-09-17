#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <config.h>

#define ARRAY_LENGTH(a) (sizeof((a))/sizeof((a)[0]))

const char *keys[] = {
    "Whiskey",
    "Age",
    "Distillery"
};


void *run(void *__restrict path)
{
    config_t *config;
    int i;
    char *data;
    
    config = config_open(path);
    if(!config)
        return NULL;
    
    for(i = 0; i < ARRAY_LENGTH(keys); ++i) {
        data = config_lookup(config, keys[i]);
        fprintf(stdout, "%s: %s -> %s\n", (char *)path, keys[i], data);
    }
    
    config_close(config);
    
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