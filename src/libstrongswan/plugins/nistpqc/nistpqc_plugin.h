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

/**
 * @defgroup nistpqc_p nistpqc
 * @ingroup plugins
 *
 * @defgroup nistpqc_plugin nistpqc_plugin
 * @{ @ingroup nistpqc_p
 */

#ifndef NISTPQC_PLUGIN_H_
#define NISTPQC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct nistpqc_plugin_t nistpqc_plugin_t;

/**
 * Plugin implementing NIST_PQC-base key exchange
 */
struct nistpqc_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** NISTPQC_PLUGIN_H_ @}*/
