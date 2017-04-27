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
 * @defgroup ntruprime_ke ntruprime_ke
 * @{ @ingroup ntruprime_p
 */

#ifndef NTRU_PRIME_KE_H_
#define NTRU_PRIME_KE_H_

typedef struct ntruprime_ke_t ntruprime_ke_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using
 * streamlined NTRU prime encryption
 */
struct ntruprime_ke_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new ntruprime_ke_t object.
 *
 * @param group			NTRU prime group number to use
 * @param g				not used
 * @param p				not used
 * @return				ntruprime_ke_t object, NULL if not supported
 */
ntruprime_ke_t* ntruprime_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p);

#endif /** NTRU_PRIME_KE_H_ @}*/
