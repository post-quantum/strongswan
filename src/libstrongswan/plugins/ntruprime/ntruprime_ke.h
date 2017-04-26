/*
 * Copyright (C) 2017 CJ Tjhai
 * Post-Quantum
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup ntru_prime_ke ntru_prime_ke
 * @{ @ingroup ntru_prime_p
 */

#ifndef NTRU_PRIME_KE_H_
#define NTRU_PRIME_KE_H_

typedef struct ntru_prime_ke_t ntru_prime_ke_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using
 * streamlined NTRU prime encryption
 */
struct ntru_prime_ke_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new ntru_prime_ke_t object.
 *
 * @param group			NTRU prime group number to use
 * @param g				not used
 * @param p				not used
 * @return				ntru_prime_ke_t object, NULL if not supported
 */
ntru_prime_ke_t* ntru_prime_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p);

#endif /** NTRU_PRIME_KE_H_ @}*/
