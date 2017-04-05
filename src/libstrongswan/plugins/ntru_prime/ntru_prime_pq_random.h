#ifndef _PQ_RANDOM_H
#define _PQ_RANDOM_H

#include <stdint.h>
#include <stdlib.h>

#define PQ_RANDOM_FIPS_NOT_SUPPORTED         -2
#define PQ_RANDOM_METHOD_NOT_SUPPORTED       -1
#define PQ_RANDOM_ERROR                       0
#define PQ_RANDOM_OK                          1

int pq_rand_init();

int pq_rand_deinit();

int pq_rand_reseed(const uint8_t *seed, size_t seed_size);

int pq_rand_bytes(uint8_t *buf, size_t buf_size);

int pq_rand_uint32_at_most(uint32_t max, uint32_t *rnd);

unsigned long pq_rand_get_last_error_code();

#endif /* _PQ_RANDOM_H */
