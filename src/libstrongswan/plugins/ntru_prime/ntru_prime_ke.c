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

#include "ntru_prime.h"
#include "ntru_prime_ke.h"
#include "ntru_prime_pq_random.h"

#include <crypto/diffie_hellman.h>
#include <utils/debug.h>

typedef struct private_ntru_prime_ke_t private_ntru_prime_ke_t;

 /**
 * Private data of an ntru_prime_ke_t object.
 */
struct private_ntru_prime_ke_t {
	/**
	 * Public ntru_prime_ke_t interface.
	 */
	ntru_prime_ke_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Cryptographical strength in bits of the NTRU Prime Parameter Set
	 */
	uint32_t strength;

	/**
	 * NTRU Prime Public Key
	 */
	chunk_t pubkey;

	/**
	 * NTRU Prime Private Key
	 */
	chunk_t privkey;

    /**
     * NTRU prime ciphertext size
     */
    size_t ciphertext_size;

	/**
	 * NTRU Prime encrypted shared secret
	 */
	chunk_t ciphertext;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * True if peer is responder
	 */
	bool responder;

	/**
	 * True if shared secret is computed
	 */
	bool computed;
};

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_ntru_prime_ke_t *this, chunk_t *value)
{
    ntru_prime *ntru_prime = NULL;
	*value = chunk_empty;

	if (this->responder)
	{
		if (this->ciphertext.len)
		{
			*value = chunk_clone(this->ciphertext);
            this->ciphertext_size = this->ciphertext.len;
		}
	}
	else
	{
		if (!this->pubkey.ptr)
		{
			/* generate a random NTRU Prime public/private key pair */
            ntru_prime = init_ntru_prime();
            if (!ntru_prime)
            {
                DBG1(DBG_LIB, "NTRU prime keypair generation failed");
				return FALSE;
            }
            this->pubkey = chunk_clone(chunk_create(ntru_prime->public_key,
                                        ntru_prime->public_key_size));
            this->privkey = chunk_clone(chunk_create(ntru_prime->private_key,
                                         ntru_prime->private_key_size));
            this->ciphertext_size = ntru_prime_ciphertext_size(ntru_prime);
            free_ntru_prime(ntru_prime);
		}
		*value = chunk_clone(this->pubkey);
		DBG3(DBG_LIB, "NTRU prime public key: %B", value);
	}
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_ntru_prime_ke_t *this, chunk_t *secret)
{
	if (!this->computed || !this->shared_secret.len)
	{
		*secret = chunk_empty;
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);

	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_ntru_prime_ke_t *this, chunk_t value)
{
	if (this->privkey.ptr)
	{
		/* initiator decrypting shared secret */
		if (value.len == 0)
		{
			DBG1(DBG_LIB, "empty NTRU prime ciphertext");
			return FALSE;
		}
		DBG3(DBG_LIB, "NTRU prime ciphertext: %B", &value);

		/* decrypt the shared secret */
        this->shared_secret = chunk_alloc(ntru_prime_kem_key_size());
        if (!ntru_prime_decapsulate(this->privkey.ptr, this->privkey.len,
                                    value.ptr, this->shared_secret.ptr))
		{
            chunk_clear(&this->shared_secret);
			DBG1(DBG_LIB, "NTRU prime decryption of shared secret failed");
			return FALSE;
		}
		this->computed = TRUE;
	}
	else
	{
		/* responder generating and encrypting the shared secret */
		this->responder = TRUE;

        this->pubkey = chunk_clone(value);

		DBG3(DBG_LIB, "NTRU prime public key: %B", &value);
        if (!this->pubkey.ptr)
		{
			return FALSE;
		}
        /**
         * FIXME: Perform error checking on the public-key
		if (pubkey->get_id(pubkey) != this->param_set->id)
		{
			DBG1(DBG_LIB, "received NTRU public key with wrong OUI");
			pubkey->destroy(pubkey);
			return FALSE;
		}
        */

		/* shared secret size is the same as NTRU prime KEM key size */
		this->shared_secret = chunk_alloc(ntru_prime_kem_key_size());

		/* generate the random shared secret */
        if (pq_rand_bytes(this->shared_secret.ptr,
                          this->shared_secret.len) != PQ_RANDOM_OK)
		{
			DBG1(DBG_LIB, "generation of shared secret failed");
			chunk_clear(&this->shared_secret);
			return FALSE;
		}
		this->computed = TRUE;

        /* Allocate a buffer to contain our ciphertext */
        this->ciphertext_size = ntru_prime_ciphertext_size_from_public_key(this->pubkey.ptr, this->pubkey.len);
        this->ciphertext = chunk_alloc(this->ciphertext_size);
        DBG3(DBG_LIB, "NTRU prime ciphertext size: %u", this->ciphertext_size);

		/* encrypt the shared secret */
        if (!ntru_prime_encapsulate(this->pubkey.ptr,
                                    this->pubkey.len,
                                    this->ciphertext.ptr,
                                    this->shared_secret.ptr))
		{
            chunk_clear(&this->ciphertext);
            chunk_clear(&this->shared_secret);
			DBG1(DBG_LIB, "NTRU prime encryption of shared secret failed");
			return FALSE;
		}
		DBG3(DBG_LIB, "NTRU prime ciphertext: %B", &this->ciphertext);
	}
	return this->computed;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_ntru_prime_ke_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_ntru_prime_ke_t *this)
{
    chunk_clear(&this->privkey);
    chunk_clear(&this->pubkey);
	chunk_free(&this->ciphertext);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header.
 */
ntru_prime_ke_t *ntru_prime_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p)
{
	private_ntru_prime_ke_t *this;
    const int strength = 129;

	DBG1(DBG_LIB, "%u-bit NTRU prime", strength);

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
		.group = group,
		.strength = strength,
		.pubkey = chunk_empty,
		.privkey = chunk_empty,
		.ciphertext_size = 0,
		.ciphertext = chunk_empty,
		.shared_secret = chunk_empty,
		.computed = FALSE,
	);

	return &this->public;
}
