#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#include "macro.h"
#include "random.h"

struct random *random_new(void)
{
    struct random *rand;
    int err;
    
    rand = malloc(sizeof(*rand));
    if(!rand)
        return NULL;
    
    err = random_init(rand);
    if(err < 0) {
        free(rand);
        return NULL;
    }
    
    return rand;
}

void random_delete(struct random *__restrict rand)
{
    random_destroy(rand);
    free(rand);
}

int random_init(struct random *__restrict rand)
{
    ssize_t err;
    unsigned int num;
    int i, fd;
    
    fd = open("/dev/urandom", O_RDONLY);
    if(!fd)
        return -errno;

    for(i = 0; i < ARRAY_SIZE(rand->x); ++i) {
        err = read(fd, &num, sizeof(num));
        if(err < 0) {
            err = -errno;
            close(fd);
            return err;
        }
        
        rand->seed[i] = num;
        rand->x[i]    = num;
    }
    
    close(fd);
    
    return 0;
}

void random_destroy(struct random *__restrict rand)
{
    memset(rand->x, 0, sizeof(rand->x));
}

int random_set_seed(struct random *__restrict rand, 
                  const unsigned int *a,
                  unsigned int size)
{
    int i;
    
    for(i = 0; i < ARRAY_SIZE(rand->x) && i < size; ++i) {
        rand->seed[i] = a[i];
        rand->x[i]    = a[i];
    }
    
    if((rand->seed[0] | rand->seed[1] | rand->seed[2] | rand->seed[3]) == 0)
        return -EINVAL;
    
    return 0;
}

int random_get_seed(const struct random *__restrict rand, 
                    unsigned int *__restrict a, 
                    unsigned int size)
{
    int i;
    
    if(size < ARRAY_SIZE(rand->x))
        return -EINVAL;
    
    for(i = 0;  i < ARRAY_SIZE(rand->x); ++i)
        a[i] = rand->seed[i];
    
    return 0;
}

unsigned int random_uint(struct random *__restrict rand)
{
    uint64_t t;
    
    rand->x[0] = 69069 * rand->x[0] + 12345;
    
    rand->x[1] ^= rand->x[1] << 13;
    rand->x[1] ^= rand->x[1] >> 17;
    rand->x[1] ^= rand->x[1] << 5;
    
    t = 698769069ULL * rand->x[2] + rand->x[3];
    rand->x[3] = t >> 32;
    rand->x[2] = (uint32_t) t;
    
    return rand->x[0] + rand->x[1] + rand->x[2];
}

unsigned int random_uint_range(struct random *__restrict rand, 
                               unsigned int min,
                               unsigned int max)
{
    return (random_uint(rand) % (max - min + 1)) + min;
}