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

#include "nistpqc_plugin.h"
#include "nistpqc_ke.h"

#include <library.h>

typedef struct private_nistpqc_plugin_t private_nistpqc_plugin_t;

/**
 * private data of nistpqc_plugin
 */
struct private_nistpqc_plugin_t {

	/**
	 * public functions
	 */
	nistpqc_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_nistpqc_plugin_t *this)
{
	return "nistpqc";
}

METHOD(plugin_t, get_features, int,
	private_nistpqc_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
#ifdef QSKE
        PLUGIN_REGISTER(QS, nistpqc_ke_create),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_NEWHOPE512CCA),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_KYBER512),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_NTRULPR4591761),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_NTRUKEM443),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_SIKEP503),
            PLUGIN_PROVIDE(QS, QS_NIST_PQC_LEDAKEM128SLN02),
#else
		PLUGIN_REGISTER(DH, nistpqc_ke_create),
            PLUGIN_PROVIDE(DH, NIST_PQC_NEWHOPE512CCA),
            PLUGIN_PROVIDE(DH, NIST_PQC_KYBER512),
            PLUGIN_PROVIDE(DH, NIST_PQC_NTRULPR4591761),
            PLUGIN_PROVIDE(DH, NIST_PQC_NTRUKEM443),
            PLUGIN_PROVIDE(DH, NIST_PQC_SIKEP503),
            PLUGIN_PROVIDE(DH, NIST_PQC_LEDAKEM128SLN02),
#endif
	};
	*features = f;

	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_nistpqc_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *nistpqc_plugin_create()
{
	private_nistpqc_plugin_t *this;

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
