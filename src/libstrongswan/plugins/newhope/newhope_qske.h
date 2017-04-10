/*
 * Copyright (C) 2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup newhope_qske newhope_qske
 * @{ @ingroup newhope_p
 */

#ifndef NEWHOPE_QSKE_H_
#define NEWHOPE_QSKE_H_

#ifdef QSKE

typedef struct newhope_qske_t newhope_qske_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using the New Hope algorithm
 */
struct newhope_qske_t {

	/**
	 * Implementation of the QSKE interface.
	 */
	quantum_safe_t qs;
};

/**
 * Creates a new newhope_qske_t object.
 *
 * @param group			not used
 * @param g				not used
 * @param p				not used
 * @return				newhope_qske_t object, NULL if not supported
 */
newhope_qske_t *newhope_qske_create(quantum_safe_group_t group, chunk_t g, chunk_t p);

#endif

#endif /** NEWHOPE_QSKE_H_ @}*/

