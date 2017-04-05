/**
 *  ntru_prime.h
 *  ntrup
 *
 *  Created by CJ Tjhai on 06/02/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#ifndef _NTRU_PRIME_H
#define _NTRU_PRIME_H

#include <stddef.h>
#include <stdint.h>
/**
 *  NTRU Prime data structure
 *   - p : a prime number
 *   - q : another prime number
 *   - t : a positive integer, such that q >= 48t + 1
 *   - public_key: a buffer containing the public-key
 *   - public_key_size : the size of the public-key in bytes
 *   - private_key: a buffer containing the private-key
 *   - private_key_size : the size of the private-key in bytes
 *   - priv : private data-structure
 **/
typedef struct {
    uint32_t p, q, t;
    uint8_t *public_key;
    uint8_t *private_key;
    size_t public_key_size;
    size_t private_key_size;
    void *priv;
} ntru_prime;

/**
 *  Initialises NTRU prime with parameters p = 547,
 *  q = 3001 and t = 62. This set of parameters is
 *  believed to give 129-bit of security.
 *
 *  @return a pointer to ntru_prime data structure on success,
 *          NULL otherwise
 **/
ntru_prime* init_ntru_prime();

/**
 *  Deallocate NTRU prime object
 *
 *  @param[in] ntrup a pointer to ntru_prime object
 **/
void free_ntru_prime(ntru_prime *ntrup);

/**
 *  Returns the size of NTRU prime KEM key in bytes
 *
 *  @return the key size in bytes
 **/
int ntru_prime_kem_key_size();

/**
 *  Returns the size of NTRU prime ciphertext in bytes
 *
 *  @param[in] ntrup a pointer to ntru_prime object
 *  @return the ciphertext size in bytes
 **/
int ntru_prime_ciphertext_size(const ntru_prime *ntrup);

/**
 *  Returns the size of NTRU prime ciphertext in bytes
 *  given a piece of buffer containing the public-key
 *
 *  @param[in] public_key       The public-key buffer
 *  @param[in] public_key_size  The size of the public-key buffer
 *  @return the ciphertext size in bytes
 **/
int ntru_prime_ciphertext_size_from_public_key(const uint8_t *public_key,
                                               size_t public_key_size);
/**
 *  Performs KEM encapsulation with NTRU prime
 *
 *  @param[in]  public_key      The NTRU prime public-key
 *  @param[in]  public_key_size The size of NTRU prime public-key in bytes
 *  @param[out] ciphertext      The pointer to ciphertext
 *  @param[out] key             The pointer to the KEM key
 *  @return 1 on success, 0 otherwise
 **/
int ntru_prime_encapsulate(const uint8_t *public_key,
                           size_t public_key_size,
                           uint8_t *ciphertext,
                           uint8_t *key);

/**
 *  Performs KEM decapsulation with NTRU prime
 *
 *  @param[in]  private_key      The NTRU prime private-key
 *  @param[in]  private_key_size The size of NTRU prime private-key in bytes
 *  @param[in]  ciphertext       The pointer to the input ciphertext
 *  @param[out] key              The pointer to the KEM key
 *  @return 1 on success, 0 otherwise
 **/
int ntru_prime_decapsulate(const uint8_t *private_key,
                           size_t private_key_size,
                           const uint8_t *ciphertext,
                           uint8_t *key);

#endif /* _NTRU_PRIME_H */
