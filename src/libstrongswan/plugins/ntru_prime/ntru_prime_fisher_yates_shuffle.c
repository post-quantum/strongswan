#include "ntru_prime_pq_random.h"

void fisher_yates_shuffle(uint32_t *buffer, size_t buffer_size)
{
    uint32_t index, swap;
    int i = (int)buffer_size - 1;
    while (i) {
        pq_rand_uint32_at_most(i, &index);
        swap = buffer[index];
        buffer[index] = buffer[i];
        buffer[i] = swap;
        --i;
    }
}
