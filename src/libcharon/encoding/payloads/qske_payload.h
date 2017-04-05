/*
 * Copyright (C) 2016 Post-Quantum
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup qske_payload qske_payload
 * @{ @ingroup payloads
 */

#ifndef QSKE_PAYLOAD_H_
#define QSKE_PAYLOAD_H_

typedef struct qske_payload_t qske_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_substructure.h>
#include <collections/linked_list.h>
#include <crypto/diffie_hellman.h>

/**
 * Class representing an IKEv1 or IKEv2 quantum-safe key exchange payload.
 */
struct qske_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Returns the quantum-safe key exchange data of this QSKE payload.
	 *
	 * @return 		chunk_t pointing to internal data
	 */
	chunk_t (*get_key_exchange_data) (qske_payload_t *this);

	/**
	 * Gets the Diffie-Hellman Group Number of this KE payload (IKEv2 only).
	 * @note This is a workaround, obviously in quantum-safe context,
	 * Diffie-Hellman should not be used anymore.
	 *
	 * @return 					DH Group Number of this payload
	 */
	diffie_hellman_group_t (*get_dh_group_number) (qske_payload_t *this);

	/**
	 * Appends the key exchange data from the secondary payload to this one
	 */
	void (*append_secondary_qske_payload) (qske_payload_t *this, qske_payload_t *secondary);

	/**
	 * Destroys a qske_payload_t object.
	 */
	void (*destroy) (qske_payload_t *this);
};

/**
 * Creates an empty qske_payload_t object.
 *
 * @param type		PLV2_KEY_EXCHANGE or PLV1_KEY_EXCHANGE
 * @return			qske_payload_t object
 */
qske_payload_t *qske_payload_create(payload_type_t type);

/**
 * Creates one or more qske_payload_t from a diffie_hellman_t.
 * (large qs key_exchange_data means we need multiple payloads)
 *
 * @param type		PLV2_KEY_EXCHANGE or PLV1_KEY_EXCHANGE
 * @param dh		diffie hellman object containing group and key
 * @param payloads	points to returned array of qske_payload_t*
 * @return 			number of payloads in the array, 0 on error
 */
int qske_payload_create_from_diffie_hellman(payload_type_t type,
								  						diffie_hellman_t *dh, 
														  qske_payload_t ***payloads);

#endif /** QSKE_PAYLOAD_H_ @}*/
