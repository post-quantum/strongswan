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

#include "nistpqc_ke.h"
#include <nistpqc/api.h>
#include <crypto/diffie_hellman.h>
#include <utils/debug.h>

typedef struct private_nistpqc_ke_t private_nistpqc_ke_t;

/**
 * Private data of an newhope_ke_t object.
 */
struct private_nistpqc_ke_t {

	/**
	 * Public newhope_ke_t interface.
	 */
	nistpqc_ke_t public;

    /**
     * NIST PQC cipher algorithm
     */
#ifdef QSKE
    quantum_safe_group_t group;
#else
	diffie_hellman_group_t group;
#endif
	/**
	 * True if peer is responder
	 */
	bool responder;

	/**
	 * True if shared secret is computed
	 */
	bool computed;

	/**
	 * Public key
	 */
	chunk_t public_key;

	/**
	 * Private key
	 */
	chunk_t private_key;

	/**
	 * Encrypted shared secret
	 */
	chunk_t ciphertext;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * NIST PQC object
	 */
	nistpqc_t nistpqc;
};

#ifdef QSKE
#define METHOD_KE(name, ret, ...) METHOD(quantum_safe_t, name, ret, private_nistpqc_ke_t *this, ##__VA_ARGS__)
#else
#define METHOD_KE(name, ret, ...) METHOD(diffie_hellman_t, name, ret, private_nistpqc_ke_t *this, ##__VA_ARGS__)
#endif

METHOD_KE(get_my_public_value, bool, chunk_t *value)
{
	*value = chunk_empty;

	if (this->responder) 
	{
		if (this->ciphertext.len)
		{
			*value = chunk_clone(this->ciphertext);
		}
	}
	else
	{
		if (!this->public_key.ptr || this->public_key.len == 0)
		{
			/**
			 * Generate a random public/private key-pair
			 */
			this->public_key  = chunk_alloc(this->nistpqc.public_key_size());
			this->private_key = chunk_alloc(this->nistpqc.private_key_size());
			if (this->nistpqc.keypair(this->public_key.ptr, this->private_key.ptr))
			{
				DBG1(DBG_LIB, "NIST_PQC (%s) key-pair generation failed", 
					this->nistpqc.algorithm_name());

				return FALSE;
			}
		}
		*value = chunk_clone(this->public_key);
		DBG4(DBG_LIB, "NIST_PQC (%s) public-key: %B", this->nistpqc.algorithm_name(), value);
	}

	return TRUE;
}

METHOD_KE(get_shared_secret, bool, chunk_t *secret)
{
	if (!this->computed || this->shared_secret.len == 0)
	{
		*secret = chunk_empty;

		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);

	return TRUE;
}

METHOD_KE(set_other_public_value, bool, chunk_t value)
{
	if (this->private_key.ptr && this->private_key.len > 0)
	{
		/**
		 * At the initiator end, decrypting a shared secret
		 */
		if (value.len == 0)
		{
			DBG1(DBG_LIB, "Empty NIST_PQC (%s) ciphertext", this->nistpqc.algorithm_name());

			return FALSE;
		}
		DBG4(DBG_LIB, "NIST_PQC (%s) ciphertext: %B", this->nistpqc.algorithm_name(), &value);

		/**
		 * Perform shared secret decryption
		 */
		this->shared_secret = chunk_alloc(this->nistpqc.shared_secret_size());
		if (this->nistpqc.dec(this->shared_secret.ptr, value.ptr, this->private_key.ptr))
		{
			DBG1(DBG_LIB, "NIST_PQC (%s) decryption of shared secret failed", 
				this->nistpqc.algorithm_name());

			return FALSE;
		}
		this->computed = TRUE;
	}
	else
	{
		/**
		 * At the responder end, generate a shared secret and encrypt it
		 */
		this->responder = TRUE;

		this->public_key = chunk_clone(value);
		DBG4(DBG_LIB, "NIST_PQC (%s) public-key: %B", this->nistpqc.algorithm_name(), &value);

		/**
		 * Generate a shared-secret randomly
		 */
		this->shared_secret = chunk_alloc(this->nistpqc.shared_secret_size());
		if (randombytes(this->shared_secret.ptr, this->shared_secret.len))
		{
			DBG1(DBG_LIB, "Generation of shared-secret failed");
			chunk_clear(&this->shared_secret);

			return FALSE;
		}
		this->computed = TRUE;

		/**
		 * Encrypt the shared secret
		 */
		this->ciphertext = chunk_alloc(this->nistpqc.ciphertext_size());
		if (this->nistpqc.enc(this->ciphertext.ptr, this->shared_secret.ptr, this->public_key.ptr))
		{
			DBG1(DBG_LIB, "NIST_PQC (%s) encryption of shared-secret failed", 
				this->nistpqc.algorithm_name());

			return FALSE;
		}
		DBG4(DBG_LIB, "NIST_PQC (%s) ciphertext: %B", 
			this->nistpqc.algorithm_name(), &this->ciphertext);
	}

	return this->computed;
}

#ifdef QSKE
METHOD_KE(get_qs_group, quantum_safe_group_t)
{
	return this->group;
}
#else
METHOD_KE(get_dh_group, diffie_hellman_group_t,
{
	return this->group;
}
#endif

METHOD_KE(destroy, void)
{
	chunk_clear(&this->shared_secret);
	chunk_free(&this->ciphertext);
	chunk_free(&this->private_key);
	chunk_free(&this->public_key);
	free(this);
}

/*
 * Described in header.
 */
#ifdef QSKE
nistpqc_ke_t *nistpqc_ke_create(quantum_safe_group_t group, chunk_t g, chunk_t p)
#else
nistpqc_ke_t *nistpqc_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p)
#endif
{
	private_nistpqc_ke_t *this;

	INIT(this,
		.public = {
#ifdef QSKE
			.qs = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_qs_group = _get_qs_group,
				.destroy = _destroy,
			},
#else
		 .dh = {
			 .get_shared_secret = _get_shared_secret,
			 .set_other_public_value = _set_other_public_value,
			 .get_my_public_value = _get_my_public_value,
			 .get_dh_group = _get_dh_group,
			 .destroy = _destroy,
		 },
#endif
		},
		.group = group,
		.public_key = chunk_empty,
		.private_key = chunk_empty,
		.ciphertext = chunk_empty,
		.shared_secret = chunk_empty,
		.computed = FALSE,
		.responder = FALSE
	);

	nistpqc_init(&this->nistpqc, (nistpqc_cipher_t)(group - NIST_PQC_NEWHOPE512CCA + 1));

	return &this->public;
}
