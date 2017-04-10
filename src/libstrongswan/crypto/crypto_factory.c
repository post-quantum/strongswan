/*
 * Copyright (C) 2013-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "crypto_factory.h"

#include <utils/debug.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <crypto/crypto_tester.h>
#include <utils/test.h>

const char *default_plugin_name = "default";

typedef struct entry_t entry_t;

struct entry_t {
	/**
	 * algorithm
	 */
	u_int algo;

	/**
	 * plugin that registered this algorithm
	 */
	const char *plugin_name;

	/**
	 * benchmarked speed
	 */
	u_int speed;

	/**
	 * constructor
	 */
	union {
		crypter_constructor_t create_crypter;
		aead_constructor_t create_aead;
		signer_constructor_t create_signer;
		hasher_constructor_t create_hasher;
		prf_constructor_t create_prf;
		xof_constructor_t create_xof;
		rng_constructor_t create_rng;
		nonce_gen_constructor_t create_nonce_gen;
		dh_constructor_t create_dh;
#ifdef QSKE
		qs_constructor_t create_qs;
#endif
		void *create;
	};
};

typedef struct private_crypto_factory_t private_crypto_factory_t;

/**
 * private data of crypto_factory
 */
struct private_crypto_factory_t {

	/**
	 * public functions
	 */
	crypto_factory_t public;

	/**
	 * registered crypters, as entry_t
	 */
	linked_list_t *crypters;

	/**
	 * registered aead transforms, as entry_t
	 */
	linked_list_t *aeads;

	/**
	 * registered signers, as entry_t
	 */
	linked_list_t *signers;

	/**
	 * registered hashers, as entry_t
	 */
	linked_list_t *hashers;

	/**
	 * registered prfs, as entry_t
	 */
	linked_list_t *prfs;

	/**
	 * registered xofs, as entry_t
	 */
	linked_list_t *xofs;

	/**
	 * registered rngs, as entry_t
	 */
	linked_list_t *rngs;

	/**
	 * registered nonce generators, as entry_t
	 */
	linked_list_t *nonce_gens;

	/**
	 * registered diffie hellman, as entry_t
	 */
	linked_list_t *dhs;

#ifdef QSKE
	/**
	 * registered quantum-safe key exchange providers, as entry_t
	 */
	linked_list_t *qss;
#endif

	/**
	 * test manager to test crypto algorithms
	 */
	crypto_tester_t *tester;

	/**
	 * whether to test algorithms during registration
	 */
	bool test_on_add;

	/**
	 * whether to test algorithms on each crypto primitive construction
	 */
	bool test_on_create;

	/**
	 * run algorithm benchmark during registration
	 */
	bool bench;

	/**
	 * Number of failed test vectors during "add".
	 */
	u_int test_failures;

	/**
	 * rwlock to lock access to modules
	 */
	rwlock_t *lock;
};

METHOD(crypto_factory_t, create_crypter, crypter_t*,
	private_crypto_factory_t *this, encryption_algorithm_t algo,
	size_t key_size)
{
	enumerator_t *enumerator;
	entry_t *entry;
	crypter_t *crypter = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->crypters->create_enumerator(this->crypters);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_crypter(this->tester, algo, key_size,
											entry->create_crypter, NULL,
											default_plugin_name))
			{
				continue;
			}
			crypter = entry->create_crypter(algo, key_size);
			if (crypter)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return crypter;
}

METHOD(crypto_factory_t, create_aead, aead_t*,
	private_crypto_factory_t *this, encryption_algorithm_t algo,
	size_t key_size, size_t salt_size)
{
	enumerator_t *enumerator;
	entry_t *entry;
	aead_t *aead = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->aeads->create_enumerator(this->aeads);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_aead(this->tester, algo, key_size,
										 salt_size, entry->create_aead, NULL,
										 default_plugin_name))
			{
				continue;
			}
			aead = entry->create_aead(algo, key_size, salt_size);
			if (aead)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return aead;
}

METHOD(crypto_factory_t, create_signer, signer_t*,
	private_crypto_factory_t *this, integrity_algorithm_t algo)
{
	enumerator_t *enumerator;
	entry_t *entry;
	signer_t *signer = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->signers->create_enumerator(this->signers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_signer(this->tester, algo,
										   entry->create_signer, NULL,
										   default_plugin_name))
			{
				continue;
			}
			signer = entry->create_signer(algo);
			if (signer)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return signer;
}

METHOD(crypto_factory_t, create_hasher, hasher_t*,
	private_crypto_factory_t *this, hash_algorithm_t algo)
{
	enumerator_t *enumerator;
	entry_t *entry;
	hasher_t *hasher = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->hashers->create_enumerator(this->hashers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_hasher(this->tester, algo,
										   entry->create_hasher, NULL,
										   default_plugin_name))
			{
				continue;
			}
			hasher = entry->create_hasher(entry->algo);
			if (hasher)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return hasher;
}

METHOD(crypto_factory_t, create_prf, prf_t*,
	private_crypto_factory_t *this, pseudo_random_function_t algo)
{
	enumerator_t *enumerator;
	entry_t *entry;
	prf_t *prf = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->prfs->create_enumerator(this->prfs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_prf(this->tester, algo,
										entry->create_prf, NULL,
										default_plugin_name))
			{
				continue;
			}
			prf = entry->create_prf(algo);
			if (prf)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return prf;
}

METHOD(crypto_factory_t, create_xof, xof_t*,
	private_crypto_factory_t *this, ext_out_function_t algo)
{
	enumerator_t *enumerator;
	entry_t *entry;
	xof_t *xof = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->xofs->create_enumerator(this->xofs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == algo)
		{
			if (this->test_on_create &&
				!this->tester->test_xof(this->tester, algo,
										entry->create_xof, NULL,
										default_plugin_name))
			{
				continue;
			}
			xof = entry->create_xof(algo);
			if (xof)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return xof;
}

METHOD(crypto_factory_t, create_rng, rng_t*,
	private_crypto_factory_t *this, rng_quality_t quality)
{
	enumerator_t *enumerator;
	entry_t *entry;
	rng_t *rng = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->rngs->create_enumerator(this->rngs);
	while (enumerator->enumerate(enumerator, &entry))
	{	/* find the best matching quality, but at least as good as requested */
		if (entry->algo >= quality)
		{
			if (this->test_on_create &&
				!this->tester->test_rng(this->tester, quality,
										entry->create_rng, NULL,
										default_plugin_name))
			{
				continue;
			}
			rng = entry->create_rng(quality);
			if (rng)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return rng;
}

METHOD(crypto_factory_t, create_nonce_gen, nonce_gen_t*,
	private_crypto_factory_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	nonce_gen_t *nonce_gen = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->nonce_gens->create_enumerator(this->nonce_gens);
	while (enumerator->enumerate(enumerator, &entry))
	{
		nonce_gen = entry->create_nonce_gen();
		if (nonce_gen)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return nonce_gen;
}

METHOD(crypto_factory_t, create_dh, diffie_hellman_t*,
	private_crypto_factory_t *this, diffie_hellman_group_t group, ...)
{
	enumerator_t *enumerator;
	entry_t *entry;
	va_list args;
	chunk_t g = chunk_empty, p = chunk_empty;
	diffie_hellman_t *diffie_hellman = NULL;

	if (group == MODP_CUSTOM)
	{
		va_start(args, group);
		g = va_arg(args, chunk_t);
		p = va_arg(args, chunk_t);
		va_end(args);
	}

	this->lock->read_lock(this->lock);
	enumerator = this->dhs->create_enumerator(this->dhs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == group)
		{
			if (this->test_on_create && group != MODP_CUSTOM &&
				!this->tester->test_dh(this->tester, group,
								entry->create_dh, NULL, default_plugin_name))
			{
				continue;
			}
			diffie_hellman = entry->create_dh(group, g, p);
			if (diffie_hellman)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return diffie_hellman;
}

#ifdef QSKE
METHOD(crypto_factory_t, create_qs, quantum_safe_t*,
	private_crypto_factory_t *this, quantum_safe_group_t group, ...)
{
	enumerator_t *enumerator;
	entry_t *entry;
	quantum_safe_t *qs = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->qss->create_enumerator(this->qss);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->algo == group)
		{
/* TODO: implement testing			
			if (this->test_on_create &&
				!this->tester->test_dh(this->tester, group,
								entry->create_qs, NULL, default_plugin_name))
			{
				continue;
			}*/
			qs = entry->create_qs(group);
			if (qs)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return qs;
}
#endif

/**
 * Insert an algorithm entry to a list
 *
 * Entries maintain the order in which algorithms were added, unless they were
 * benchmarked and speed is provided, which then is used to order entries of
 * the same algorithm.
 * An exception are RNG entries, which are sorted by algorithm identifier.
 */
static void add_entry(private_crypto_factory_t *this, linked_list_t *list,
					  int algo, const char *plugin_name,
					  u_int speed, void *create)
{
	enumerator_t *enumerator;
	entry_t *entry, *current;
	bool sort = (list == this->rngs), found = FALSE;

	INIT(entry,
		.algo = algo,
		.plugin_name = plugin_name,
		.speed = speed,
	);
	entry->create = create;

	this->lock->write_lock(this->lock);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (sort && current->algo > algo)
		{
			break;
		}
		else if (current->algo == algo)
		{
			if (speed > current->speed)
			{
				break;
			}
			found = TRUE;
		}
		else if (found)
		{
			break;
		}
	}
	list->insert_before(list, enumerator, entry);
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_crypter, bool,
	private_crypto_factory_t *this, encryption_algorithm_t algo, size_t key_size,
	const char *plugin_name, crypter_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_crypter(this->tester, algo, key_size, create,
								   this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->crypters, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_crypter, void,
	private_crypto_factory_t *this, crypter_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->crypters->create_enumerator(this->crypters);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_crypter == create)
		{
			this->crypters->remove_at(this->crypters, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_aead, bool,
	private_crypto_factory_t *this, encryption_algorithm_t algo, size_t key_size,
	const char *plugin_name, aead_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_aead(this->tester, algo, key_size, 0, create,
								this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->aeads, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_aead, void,
	private_crypto_factory_t *this, aead_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->aeads->create_enumerator(this->aeads);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_aead == create)
		{
			this->aeads->remove_at(this->aeads, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_signer, bool,
	private_crypto_factory_t *this, integrity_algorithm_t algo,
	const char *plugin_name, signer_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_signer(this->tester, algo, create,
								  this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->signers, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_signer, void,
	private_crypto_factory_t *this, signer_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->signers->create_enumerator(this->signers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_signer == create)
		{
			this->signers->remove_at(this->signers, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_hasher, bool,
	private_crypto_factory_t *this, hash_algorithm_t algo,
	const char *plugin_name, hasher_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_hasher(this->tester, algo, create,
								  this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->hashers, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_hasher, void,
	private_crypto_factory_t *this, hasher_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->hashers->create_enumerator(this->hashers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_hasher == create)
		{
			this->hashers->remove_at(this->hashers, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_prf, bool,
	private_crypto_factory_t *this, pseudo_random_function_t algo,
	const char *plugin_name, prf_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_prf(this->tester, algo, create,
							   this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->prfs, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_prf, void,
	private_crypto_factory_t *this, prf_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->prfs->create_enumerator(this->prfs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_prf == create)
		{
			this->prfs->remove_at(this->prfs, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_xof, bool,
	private_crypto_factory_t *this, ext_out_function_t algo,
	const char *plugin_name, xof_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_xof(this->tester, algo, create,
							   this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->xofs, algo, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_xof, void,
	private_crypto_factory_t *this, xof_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->xofs->create_enumerator(this->xofs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_xof == create)
		{
			this->xofs->remove_at(this->xofs, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_rng, bool,
	private_crypto_factory_t *this, rng_quality_t quality,
	const char *plugin_name, rng_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_rng(this->tester, quality, create,
							   this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->rngs, quality, plugin_name, speed, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_rng, void,
	private_crypto_factory_t *this, rng_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->rngs->create_enumerator(this->rngs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_rng == create)
		{
			this->rngs->remove_at(this->rngs, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_nonce_gen, bool,
	private_crypto_factory_t *this, const char *plugin_name,
	nonce_gen_constructor_t create)
{
	add_entry(this, this->nonce_gens, 0, plugin_name, 0, create);
	return TRUE;
}

METHOD(crypto_factory_t, remove_nonce_gen, void,
	private_crypto_factory_t *this, nonce_gen_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->nonce_gens->create_enumerator(this->nonce_gens);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_nonce_gen == create)
		{
			this->nonce_gens->remove_at(this->nonce_gens, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(crypto_factory_t, add_dh, bool,
	private_crypto_factory_t *this, diffie_hellman_group_t group,
	const char *plugin_name, dh_constructor_t create)
{
	u_int speed = 0;

	if (!this->test_on_add ||
		this->tester->test_dh(this->tester, group, create,
							  this->bench ? &speed : NULL, plugin_name))
	{
		add_entry(this, this->dhs, group, plugin_name, 0, create);
		return TRUE;
	}
	this->test_failures++;
	return FALSE;
}

METHOD(crypto_factory_t, remove_dh, void,
	private_crypto_factory_t *this, dh_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->dhs->create_enumerator(this->dhs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_dh == create)
		{
			this->dhs->remove_at(this->dhs, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

#ifdef QSKE

METHOD(crypto_factory_t, add_qs, bool,
	private_crypto_factory_t *this, quantum_safe_group_t group,
	const char *plugin_name, qs_constructor_t create)
{
	add_entry(this, this->qss, group, plugin_name, 0, create);
	return TRUE;
}

METHOD(crypto_factory_t, remove_qs, void,
	private_crypto_factory_t *this, qs_constructor_t create)
{
	entry_t *entry;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->qss->create_enumerator(this->qss);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create_qs == create)
		{
			this->qss->remove_at(this->qss, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

#endif

/**
 * match algorithms of an entry?
 */
static bool entry_match(entry_t *a, entry_t *b)
{
	return a->algo == b->algo;
}

/**
 * check for uniqueness of an entry
 */
static bool unique_check(linked_list_t *list, entry_t **in, entry_t **out)
{
	if (list->find_first(list, (void*)entry_match, NULL, *in) == SUCCESS)
	{
		return FALSE;
	}
	*out = *in;
	list->insert_last(list, *in);
	return TRUE;
}

/**
 * create an enumerator over entry->algo in list with locking and unique check
 */
static enumerator_t *create_enumerator(private_crypto_factory_t *this,
									   linked_list_t *list, void *filter)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(
				enumerator_create_filter(
					list->create_enumerator(list), (void*)unique_check,
					linked_list_create(), (void*)list->destroy),
				filter,	this->lock, (void*)this->lock->unlock);
}

/**
 * Filter function to enumerate algorithm, not entry
 */
static bool crypter_filter(void *n, entry_t **entry, encryption_algorithm_t *algo,
						   void *i2, const char **plugin_name)
{
	*algo = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_crypter_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->crypters, crypter_filter);
}

METHOD(crypto_factory_t, create_aead_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->aeads, crypter_filter);
}

/**
 * Filter function to enumerate algorithm, not entry
 */
static bool signer_filter(void *n, entry_t **entry, integrity_algorithm_t *algo,
						  void *i2, const char **plugin_name)
{
	*algo = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_signer_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->signers, signer_filter);
}

/**
 * Filter function to enumerate algorithm, not entry
 */
static bool hasher_filter(void *n, entry_t **entry, hash_algorithm_t *algo,
						  void *i2, const char **plugin_name)
{
	*algo = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_hasher_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->hashers, hasher_filter);
}

/**
 * Filter function to enumerate algorithm, not entry
 */
static bool prf_filter(void *n, entry_t **entry, pseudo_random_function_t *algo,
					   void *i2, const char **plugin_name)
{
	*algo = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_prf_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->prfs, prf_filter);
}

/**
 * Filter function to enumerate algorithm, not entry
 */
static bool xof_filter(void *n, entry_t **entry, ext_out_function_t *algo,
					   void *i2, const char **plugin_name)
{
	*algo = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_xof_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->xofs, xof_filter);
}

/**
 * Filter function to enumerate group, not entry
 */
static bool dh_filter(void *n, entry_t **entry, diffie_hellman_group_t *group,
					  void *i2, const char **plugin_name)
{
	*group = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_dh_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->dhs, dh_filter);
}

/**
 * Filter function to enumerate strength, not entry
 */
static bool rng_filter(void *n, entry_t **entry, rng_quality_t *quality,
					   void *i2, const char **plugin_name)
{
	*quality = (*entry)->algo;
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_rng_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->rngs, rng_filter);
}

/**
 * Filter function to enumerate plugin name, not entry
 */
static bool nonce_gen_filter(void *n, entry_t **entry, const char **plugin_name)
{
	*plugin_name = (*entry)->plugin_name;
	return TRUE;
}

METHOD(crypto_factory_t, create_nonce_gen_enumerator, enumerator_t*,
	private_crypto_factory_t *this)
{
	return create_enumerator(this, this->nonce_gens, nonce_gen_filter);
}

METHOD(crypto_factory_t, add_test_vector, void,
	private_crypto_factory_t *this, transform_type_t type, void *vector)
{
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			return this->tester->add_crypter_vector(this->tester, vector);
		case AEAD_ALGORITHM:
			return this->tester->add_aead_vector(this->tester, vector);
		case INTEGRITY_ALGORITHM:
			return this->tester->add_signer_vector(this->tester, vector);
		case HASH_ALGORITHM:
			return this->tester->add_hasher_vector(this->tester, vector);
		case PSEUDO_RANDOM_FUNCTION:
			return this->tester->add_prf_vector(this->tester, vector);
		case EXTENDED_OUTPUT_FUNCTION:
			return this->tester->add_xof_vector(this->tester, vector);
		case RANDOM_NUMBER_GENERATOR:
			return this->tester->add_rng_vector(this->tester, vector);
		case DIFFIE_HELLMAN_GROUP:
			return this->tester->add_dh_vector(this->tester, vector);
		default:
			DBG1(DBG_LIB, "%N test vectors not supported, ignored",
				 transform_type_names, type);
	}
}

/**
 * Private enumerator for create_verify_enumerator()
 */
typedef struct {
	enumerator_t public;
	enumerator_t *inner;
	transform_type_t type;
	crypto_tester_t *tester;
	rwlock_t *lock;
} verify_enumerator_t;

METHOD(enumerator_t, verify_enumerate, bool,
	verify_enumerator_t *this, u_int *alg, const char **plugin, bool *valid)
{
	entry_t *entry;

	if (!this->inner->enumerate(this->inner, &entry))
	{
		return FALSE;
	}
	switch (this->type)
	{
		case ENCRYPTION_ALGORITHM:
			*valid = this->tester->test_crypter(this->tester, entry->algo, 0,
							entry->create_crypter, NULL, entry->plugin_name);
			break;
		case AEAD_ALGORITHM:
			*valid = this->tester->test_aead(this->tester, entry->algo, 0, 0,
							entry->create_aead, NULL, entry->plugin_name);
			break;
		case INTEGRITY_ALGORITHM:
			*valid = this->tester->test_signer(this->tester, entry->algo,
							entry->create_signer, NULL, entry->plugin_name);
			break;
		case HASH_ALGORITHM:
			*valid = this->tester->test_hasher(this->tester, entry->algo,
							entry->create_hasher, NULL, entry->plugin_name);
			break;
		case PSEUDO_RANDOM_FUNCTION:
			*valid = this->tester->test_prf(this->tester, entry->algo,
							entry->create_prf, NULL, entry->plugin_name);
			break;
		case EXTENDED_OUTPUT_FUNCTION:
			*valid = this->tester->test_xof(this->tester, entry->algo,
							entry->create_xof, NULL, entry->plugin_name);
			break;
		case RANDOM_NUMBER_GENERATOR:
			*valid = this->tester->test_rng(this->tester, entry->algo,
							entry->create_rng, NULL, entry->plugin_name);
			break;
		case DIFFIE_HELLMAN_GROUP:
			*valid = this->tester->test_dh(this->tester, entry->algo,
							entry->create_dh, NULL, entry->plugin_name);
			break;
		default:
			return FALSE;
	}
	*plugin = entry->plugin_name;
	*alg = entry->algo;
	return TRUE;
}

METHOD(enumerator_t, verify_destroy, void,
	verify_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	this->lock->unlock(this->lock);
	free(this);
}

METHOD(crypto_factory_t, create_verify_enumerator, enumerator_t*,
	private_crypto_factory_t *this, transform_type_t type)
{
	verify_enumerator_t *enumerator;
	enumerator_t *inner;

	this->lock->read_lock(this->lock);
	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			inner = this->crypters->create_enumerator(this->crypters);
			break;
		case AEAD_ALGORITHM:
			inner = this->aeads->create_enumerator(this->aeads);
			break;
		case INTEGRITY_ALGORITHM:
			inner = this->signers->create_enumerator(this->signers);
			break;
		case HASH_ALGORITHM:
			inner = this->hashers->create_enumerator(this->hashers);
			break;
		case PSEUDO_RANDOM_FUNCTION:
			inner = this->prfs->create_enumerator(this->prfs);
			break;
		case EXTENDED_OUTPUT_FUNCTION:
			inner = this->xofs->create_enumerator(this->xofs);
			break;
		case RANDOM_NUMBER_GENERATOR:
			inner = this->rngs->create_enumerator(this->rngs);
			break;
		case DIFFIE_HELLMAN_GROUP:
			inner = this->dhs->create_enumerator(this->dhs);
			break;
#ifdef QSKE
		case QUANTUM_SAFE_GROUP:
			inner = this->qss->create_enumerator(this->qss);
			break;
#endif
		default:
			this->lock->unlock(this->lock);
			return enumerator_create_empty();
	}
	INIT(enumerator,
		.public = {
			.enumerate = (void*)_verify_enumerate,
			.destroy = _verify_destroy,
		},
		.inner = inner,
		.type = type,
		.tester = this->tester,
		.lock = this->lock,
	);
	return &enumerator->public;
}

METHOD(crypto_factory_t, destroy, void,
	private_crypto_factory_t *this)
{
	this->crypters->destroy(this->crypters);
	this->aeads->destroy(this->aeads);
	this->signers->destroy(this->signers);
	this->hashers->destroy(this->hashers);
	this->prfs->destroy(this->prfs);
	this->xofs->destroy(this->xofs);
	this->rngs->destroy(this->rngs);
	this->nonce_gens->destroy(this->nonce_gens);
	this->dhs->destroy(this->dhs);
#ifdef QSKE
	this->qss->destroy(this->qss);
#endif
	this->tester->destroy(this->tester);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
crypto_factory_t *crypto_factory_create()
{
	private_crypto_factory_t *this;

	INIT(this,
		.public = {
			.create_crypter = _create_crypter,
			.create_aead = _create_aead,
			.create_signer = _create_signer,
			.create_hasher = _create_hasher,
			.create_prf = _create_prf,
			.create_xof = _create_xof,
			.create_rng = _create_rng,
			.create_nonce_gen = _create_nonce_gen,
			.create_dh = _create_dh,
#ifdef QSKE
			.create_qs = _create_qs,
#endif
			.add_crypter = _add_crypter,
			.remove_crypter = _remove_crypter,
			.add_aead = _add_aead,
			.remove_aead = _remove_aead,
			.add_signer = _add_signer,
			.remove_signer = _remove_signer,
			.add_hasher = _add_hasher,
			.remove_hasher = _remove_hasher,
			.add_prf = _add_prf,
			.remove_prf = _remove_prf,
			.add_xof = _add_xof,
			.remove_xof = _remove_xof,
			.add_rng = _add_rng,
			.remove_rng = _remove_rng,
			.add_nonce_gen = _add_nonce_gen,
			.remove_nonce_gen = _remove_nonce_gen,
			.add_dh = _add_dh,
			.remove_dh = _remove_dh,
#ifdef QSKE
			.add_qs = _add_qs,
			.remove_qs = _remove_qs,
#endif
			.create_crypter_enumerator = _create_crypter_enumerator,
			.create_aead_enumerator = _create_aead_enumerator,
			.create_signer_enumerator = _create_signer_enumerator,
			.create_hasher_enumerator = _create_hasher_enumerator,
			.create_prf_enumerator = _create_prf_enumerator,
			.create_xof_enumerator = _create_xof_enumerator,
			.create_dh_enumerator = _create_dh_enumerator,
			.create_rng_enumerator = _create_rng_enumerator,
			.create_nonce_gen_enumerator = _create_nonce_gen_enumerator,
			.add_test_vector = _add_test_vector,
			.create_verify_enumerator = _create_verify_enumerator,
			.destroy = _destroy,
		},
		.crypters = linked_list_create(),
		.aeads = linked_list_create(),
		.signers = linked_list_create(),
		.hashers = linked_list_create(),
		.prfs = linked_list_create(),
		.xofs = linked_list_create(),
		.rngs = linked_list_create(),
		.nonce_gens = linked_list_create(),
		.dhs = linked_list_create(),
#ifdef QSKE
		.qss = linked_list_create(),
#endif
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.tester = crypto_tester_create(),
		.test_on_add = lib->settings->get_bool(lib->settings,
								"%s.crypto_test.on_add", FALSE, lib->ns),
		.test_on_create = lib->settings->get_bool(lib->settings,
								"%s.crypto_test.on_create", FALSE, lib->ns),
		.bench = lib->settings->get_bool(lib->settings,
								"%s.crypto_test.bench", FALSE, lib->ns),
	);

	return &this->public;
}
