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

#include "ntru_prime_plugin.h"
#include "ntru_prime_ke.h"
#include "ntru_prime_qske.h"

#include <library.h>

typedef struct private_ntru_prime_plugin_t private_ntru_prime_plugin_t;

/**
 * private data of ntru_prime_plugin
 */
struct private_ntru_prime_plugin_t {

	/**
	 * public functions
	 */
	ntru_prime_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ntru_prime_plugin_t *this)
{
	return "ntru_prime";
}

METHOD(plugin_t, get_features, int,
	private_ntru_prime_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DH, ntru_prime_ke_create),
			PLUGIN_PROVIDE(DH, NTRU_PRIME_129_BIT),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
#ifdef QSKE
		PLUGIN_REGISTER(QS, ntru_prime_qske_create),
			PLUGIN_PROVIDE(QS, QS_NTRU_PRIME_129_BIT),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
#endif
	};
	*features = f;

	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ntru_prime_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ntru_prime_plugin_create()
{
	private_ntru_prime_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
