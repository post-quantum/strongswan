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

/**
 * @defgroup ntruprime_p ntruprime
 * @ingroup plugins
 *
 * @defgroup ntruprime_plugin ntruprime_plugin
 * @{ @ingroup ntruprime_p
 */

#ifndef NTRUPRIME_PLUGIN_H_
#define NTRUPRIME_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ntruprime_plugin_t ntruprime_plugin_t;

/**
 * Plugin implementing NTRU-Prime-base key exchange
 */
struct ntruprime_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** NTRUPRIME_PLUGIN_H_ @}*/
