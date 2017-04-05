/**
 *  math_utils.c
 *  ntrup
 *
 *  Created by CJ Tjhai on 02/02/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#include <stdlib.h>
#include "ntru_prime_math_utils.h"

#define DEFAULT_PRIME_DECOMPOSITION_BUFFER_SIZE 8192

uint64_t modulo_exp(uint64_t a, uint64_t n, uint64_t p)
{
    uint64_t _n = n, x = a, y = 1;
    while (_n > 0) {
        if (_n & 1)
            y = (y * x) % p;
        x = (x * x) % p;
        _n >>= 1;
    }
    return y;
}

uint64_t modulo_exp_squaring(uint64_t a, uint64_t n, uint64_t p)
{
    uint64_t _n = n, x = a;
    /**
     * Note that n = 2^k, in order to compute a^n mod p, we need
     * to square a k times. Therefore _n needs to shifted right
     * until it becomes 1
     **/
    while (_n > 1) {
        x = (x * x) % p;
        _n >>= 1;
    }
    return x;
}

uint64_t* unique_prime_decomposition(uint64_t a, size_t *size)
{
    int p, i = 0;
    uint64_t *primes = (uint64_t *)calloc(DEFAULT_PRIME_DECOMPOSITION_BUFFER_SIZE, sizeof(uint64_t));
    if (!primes)
        return NULL;
    if (!(a & 1)) {
        primes[ i++ ] = 2;
        /* Factor out 2 from a */
        do {
            a /= 2;
        } while (!(a & 1));
    }
    for (p=3; a>1 ;p+=2) {
        if (!(a % p)) {
            primes[ i++ ] = p;
            /* Factor out p from a */
            do {
                a /= p;
            } while (!(a % p));
        }
    }
    *size = i;
    return primes;
}

uint64_t primitive_root(uint64_t p, uint64_t N)
{
    int i;
    uint64_t w;
    uint64_t *primes;
    size_t size = 0;
    
    /* The condition N | (p-1) must hold */
    if ((p-1) % N)
        return 0;
    
    /* List out all unique primes that divide N */
    primes = unique_prime_decomposition(N, &size);
    
    /* Search from w = 2 */
    for (w=2; w<p; w++) {
        /* Is w^N = 1 mod p */
        if (modulo_exp(w, N, p) == 1) {
            /* Check if w^(N/t) != 1 for all primes t that divide N */
            for (i=0; i<size && modulo_exp(w, N/primes[i], p) != 1; i++);
            if (i == size) {
                free(primes);
                return w;
            }
        }
    }
    
    free(primes);
    
    return 0;
}

uint64_t modulo_inverse(uint64_t a, uint64_t p)
{
    /* See Knuth, Seminumerical Algorithm Vol 2 */
    uint64_t u1, u3, v1, v3, t1, t3, q;
    int iter;
    /* Step X1. Initialise */
    u1 = 1;
    u3 = a;
    v1 = 0;
    v3 = p;
    /* Remember odd/even iterations */
    iter = 1;
    /* Step X2. Loop while v3 != 0 */
    while (v3 != 0) {
        /* Step X3. Divide and "Subtract" */
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        /* Swap */
        u1 = v1; v1 = t1; u3 = v3; v3 = t3;
        iter = -iter;
    }
    /* Make sure u3 = gcd(u,v) == 1 */
    if (u3 != 1)
        return 0;   /* Error: No inverse exists */
    /* Ensure a positive result */
    if (iter < 0)
        return p - u1;
    return u1;
}

uint64_t modulo_inverse_prime(uint64_t a, uint64_t p)
{
    /**
     * If p is a prime, there is no subfields. Therefore
     * any nonzero elements of the field p has order p-1.
     * Consequently, given a nonzero element a, its inverse
     * is a^{p-2}.
     **/
    return modulo_exp(a, p-2, p);
}

uint32_t mod_3(uint32_t a)
{
    static const uint32_t tab[18] = {
        0, 1, 2, 0, 1, 2,
        0, 1, 2, 0, 1, 2,
        0, 1, 2, 0, 1, 2
    };
    
    a = (a & 0xFFFF) + (a >> 16);
    a = (a & 0x00FF) + (a >>  8);
    a = (a & 0x000F) + (a >>  4);
    a = (a & 0x0003) + (a >>  2);
    
    return tab[a];
}

int32_t int_mod(int32_t a, int32_t p)
{
    int32_t q = a/p;
    a = a - (q * p);
    if (a < 0)
        return a + p;
    return a;
}
