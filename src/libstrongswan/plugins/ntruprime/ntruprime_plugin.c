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

#include "ntruprime_plugin.h"
#include "ntruprime_ke.h"
#include "ntruprime_qske.h"

#include <library.h>

typedef struct private_ntruprime_plugin_t private_ntruprime_plugin_t;

/**
 * private data of ntruprime_plugin
 */
struct private_ntruprime_plugin_t {

	/**
	 * public functions
	 */
	ntruprime_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ntruprime_plugin_t *this)
{
	return "ntruprime";
}

METHOD(plugin_t, get_features, int,
	private_ntruprime_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DH, ntruprime_ke_create),
			PLUGIN_PROVIDE(DH, NTRU_PRIME_129_BIT),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
#ifdef QSKE
		PLUGIN_REGISTER(QS, ntruprime_qske_create),
			PLUGIN_PROVIDE(QS, QS_NTRU_PRIME_129_BIT),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
#endif
	};
	*features = f;

	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ntruprime_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ntruprime_plugin_create()
{
	private_ntruprime_plugin_t *this;

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
