/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup ntru_qske ntru_qske
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_QSKE_H_
#define NTRU_QSKE_H_

#ifdef QSKE

typedef struct ntru_qske_t ntru_qske_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using NTRU encryption
 */
struct ntru_qske_t {

	/**
	 * Implements QSKE interface.
	 */
	quantum_safe_t qs;
};

/**
 * Creates a new ntru_ke_t object.
 *
 * @param group			NTRU group number to use
 * @param g				not used
 * @param p				not used
 * @return				ntru_qske_t object, NULL if not supported
 */
ntru_qske_t *ntru_qske_create(quantum_safe_group_t group, chunk_t g, chunk_t p);

#endif


#endif /** NTRU_QSKE_H_ @}*/

