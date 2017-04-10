/*
 * Copyright (C) 2016 Post-Quantum
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include <stddef.h>

#include "qske_payload.h"

#include <encoding/payloads/encodings.h>

typedef struct private_qske_payload_t private_qske_payload_t;

/**
 * Private data of an qske_payload_t object.
 */
struct private_qske_payload_t {

	/**
	 * Public qske_payload_t interface.
	 */
	qske_payload_t public;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * Reserved bits
	 */
	bool reserved_bit[7];

	/**
	 * Reserved bytes
	 */
	uint8_t reserved_byte[2];

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * QS Group Number.
	 */
	quantum_safe_group_t qs_group_number;

	/**
	 * Key Exchange Data of this QSKE payload.
	 */
	chunk_t key_exchange_data;

	/**
	 * Payload type, PLV2_KEY_EXCHANGE or PLV1_KEY_EXCHANGE
	 */
	payload_type_t type;
};

/**
 * Encoding rules for IKEv2 key exchange payload.
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_qske_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_qske_payload_t, critical)		},
	/* 7 Bit reserved bits */
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_qske_payload_t, reserved_bit[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_qske_payload_t, payload_length)	},
	/* QS Group number as 16 bit field*/
	{ U_INT_16,			offsetof(private_qske_payload_t, qs_group_number)	},
	/* 2 reserved bytes */
	{ RESERVED_BYTE,	offsetof(private_qske_payload_t, reserved_byte[0])},
	{ RESERVED_BYTE,	offsetof(private_qske_payload_t, reserved_byte[1])},
	/* Quantum-Safe Key Exchange Data is from variable size */
	{ CHUNK_DATA,		offsetof(private_qske_payload_t, key_exchange_data)},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !          DH Group #           !           RESERVED            !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       Key Exchange Data                       ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

static encoding_rule_t encodings_v1[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_qske_payload_t, next_payload) 	},
	/* Reserved Byte */
	{ RESERVED_BYTE,	offsetof(private_qske_payload_t, reserved_byte[0])},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_qske_payload_t, payload_length)	},
	/* Quantum-Safe Key Exchange Data is from variable size */
	{ CHUNK_DATA,		offsetof(private_qske_payload_t, key_exchange_data)},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !    RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       Key Exchange Data                       ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/


METHOD(payload_t, verify, status_t,
	private_qske_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_qske_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_QSKEY_EXCHANGE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_qske_payload_t *this)
{
	if (this->type == PLV2_QSKEY_EXCHANGE)
	{
		return 8;
	}
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_qske_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_qske_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_qske_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_qske_payload_t *this)
{
	return this->payload_length;
}

METHOD(qske_payload_t, get_key_exchange_data, chunk_t,
	private_qske_payload_t *this)
{
	return this->key_exchange_data;
}

METHOD(qske_payload_t, get_qs_group_number, quantum_safe_group_t,
	private_qske_payload_t *this)
{
	return this->qs_group_number;
}

METHOD(qske_payload_t, append_secondary_qske_payload, void,
	private_qske_payload_t *this, qske_payload_t *secondary)
{
	chunk_t chunk = chunk_cat("cc", this->key_exchange_data, secondary->get_key_exchange_data(secondary));
	chunk_free(&this->key_exchange_data);
	this->key_exchange_data = chunk;
}

METHOD2(payload_t, qske_payload_t, destroy, void,
	private_qske_payload_t *this)
{
	free(this->key_exchange_data.ptr);
	free(this);
}

/*
 * Described in header
 */
qske_payload_t *qske_payload_create(payload_type_t type)
{
	private_qske_payload_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.get_key_exchange_data = _get_key_exchange_data,
			.get_qs_group_number = _get_qs_group_number,
			.append_secondary_qske_payload = _append_secondary_qske_payload,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.qs_group_number = QS_NONE,
		.type = type,
	);
	this->payload_length = get_header_length(this);
	return &this->public;
}

/*
 * Described in header
 */
int qske_payload_create_from_qs(payload_type_t type,
								quantum_safe_t *qs, 
								qske_payload_t ***payloads)
{
	chunk_t value;

	*payloads = NULL;
	if (!qs->get_my_public_value(qs, &value))
	{
		return 0;
	}

#if defined(PQPERF)
	printf("PQPERF: qske_payload data len = %lu\n", value.len);
#endif

	size_t max_payload_chunk_size = 65535 - offsetof(private_qske_payload_t, key_exchange_data);
	
	// Testing: enforce a small payload max size to test multipayloads
	//max_payload_chunk_size = 64;

	// Create enough payloads to hold the QS key exchange data
	int num_payloads = (value.len + max_payload_chunk_size - 1) / max_payload_chunk_size;
	int offset = 0;
	int remaining = value.len;
	if (num_payloads > 1) 
	{
		DBG0(DBG_ENC, "*** WARNING! QSKE payload splitting is likely broken **");
	}
	*payloads = (qske_payload_t**)malloc(sizeof(qske_payload_t*) * num_payloads);
	int i;
	for (i=0 ; i<num_payloads ; i++)
	{
		DBG1(DBG_ENC, "Creating QSKE mini payload %d", i);
		private_qske_payload_t* payload = (private_qske_payload_t*)qske_payload_create(type);
		payload->qs_group_number = i ? 0 : qs->get_qs_group(qs);
		int payload_chunk_len = min(max_payload_chunk_size, remaining);
		payload->key_exchange_data = chunk_alloc(payload_chunk_len);
		memcpy(payload->key_exchange_data.ptr, value.ptr + offset, payload_chunk_len);
		payload->payload_length += payload_chunk_len;
		(*payloads)[i] = &payload->public;
		offset += payload_chunk_len;
		remaining -= payload_chunk_len;
	}
	chunk_free(&value);

	return num_payloads;
}
