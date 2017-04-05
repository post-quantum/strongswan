/**
 *  math_utils.h
 *  ntrup
 *
 *  Created by CJ Tjhai on 02/02/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#ifndef math_utils_h
#define math_utils_h

#include <stdint.h>

/**
 * Modular exponentiation, a^n mod p
 *
 * Let a = (a_k, ... , a_1 , a_0)_2 be the binary representation
 * of integer a. It can be shown that
 *
 *   a^n mod p = \prod_j (a^(2^j) mod p)
 *
 * where a_j is non zero.
 *
 * @param a The base integer
 * @param n The integer exponent
 * @param p The modulo integer
 * @return a integer < p
 **/
uint64_t modulo_exp(uint64_t a, uint64_t n, uint64_t p);

/**
 * Modular exponentiation, a^n mod p, where n is a power of 2
 *
 * @param a The base integer
 * @param n The integer exponent, a power of 2
 * @param p The modulo integer
 * @return a integer < p
 **/
uint64_t modulo_exp_squaring(uint64_t a, uint64_t n, uint64_t p);

/**
 * Primitive N-th root of unity for field of size p
 *
 * @param p The field size
 * @param N The integer N, where N | (p-1)
 * @return the primitive N-th root of unity
 **/
uint64_t primitive_root(uint64_t p, uint64_t N);

/**
 * Returns unique prime decompositions of an integer.
 *
 * @param a    The input integer
 * @param size The number of unique primes
 * @return A list of unique primes from integer a
 **/
uint64_t* unique_prime_decomposition(uint64_t a, size_t *size);

/**
 * Returns the modulo inverse of a modulo p
 *
 * @param a The input integer
 * @param p The field size
 * @return Returns the inverse of a
 **/
uint64_t modulo_inverse(uint64_t a, uint64_t p);

/**
 * Returns the modulo inverse of a modulo prime p
 *
 * @param a The input integer
 * @param p The prime field size
 * @return Returns the inverse of a
 **/
uint64_t modulo_inverse_prime(uint64_t a, uint64_t p);

/**
 * Returns the modulo 3 of an integer a
 *
 * @param a The input integer
 * @return The modulo 3 o f integer a
 **/
uint32_t mod_3(uint32_t a);

int32_t int_mod(int32_t a, int32_t p);

#endif /* math_utils_h */
