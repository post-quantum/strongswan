/*
 * Copyright (C) 2018 CJ 
 * Post-Quantum Ltd
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
 * @defgroup nistpqc_ke nistpqc_ke
 * @{ @ingroup nistpqc_p
 */

#ifndef NISTPQC_KE_H_
#define NISTPQC_KE_H_

typedef struct nistpqc_ke_t nistpqc_ke_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using PQC algorithms
 * submitted to NIST PQC standardisation
 */
struct nistpqc_ke_t {

#ifdef QSKE
	/**
	 * Implementation of the QSKE interface.
	 */
	quantum_safe_t qs;
#else
	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
#endif
};

/**
 * Creates a new nistpqc_ke_t object.
 *
 * @param group			NIST PQC DH group number
 * @param g				not used
 * @param p				not used
 * @return				nistpqc_ke_t object, NULL if not supported
 */
#ifdef QSKE
nistpqc_ke_t *nistpqc_ke_create(quantum_safe_group_t group, chunk_t g, chunk_t p);
#else
nistpqc_ke_t *nistpqc_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p);
#endif

#endif /** NISTPQC_KE_H_ @}*/

