#ifndef _PRNG_H_
#define _PRNG_H_

/* KISS PRNG from George Marsaglia */

struct random {
    unsigned int x[4];
    unsigned int seed[4];
};

struct random *random_new(void);

void random_delete(struct random *__restrict rand);

int random_init(struct random *__restrict rand);

void random_destroy(struct random *__restrict rand);

int random_set_seed(struct random *__restrict rand, 
                    const unsigned int *__restrict a,
                    unsigned int size);

int random_get_seed(const struct random *__restrict rand, 
                    unsigned int *__restrict a, 
                    unsigned int size);

unsigned int random_uint(struct random *__restrict rand);

unsigned int random_uint_range(struct random *__restrict rand, 
                               unsigned int min,
                               unsigned int max);

#endif /* _PRNG_H_ */