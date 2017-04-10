/*
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

/**
 * @defgroup newhope_poly newhope_poly
 * @{ @ingroup newhope_p
 */

#ifndef NEWHOPE_POLY_H_
#define NEWHOPE_POLY_H_

#include <ntt_fft.h>
#include <library.h>

/**
 * Derive 14-bit coefficients of polynomial a from 256 bit random seed
 * using the SHAKE128 extended output function
 */
uint32_t* derive_a_poly(const ntt_fft_params_t *params, chunk_t seed);

/**
 * Pack four 14-bit coefficients into seven consecutive bytes
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |L 0 0 0 0 0 0 0|L 1 H 0 0 0 0 0|M 1 1 1 1 1 1 1|L 2 2 2 H 1 1 1|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |M 2 2 2 2 2 2 2|L 3 3 3 3 3 H 2|H 3 3 3 3 3 3 3|L 0 0 0 0 0 0 0|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
void pack_poly(const ntt_fft_params_t *params, uint8_t *x, uint32_t *p);

/**
 * Unpack seven consecutive bytes into four 14-bit coefficients
 */
uint32_t* unpack_poly(const ntt_fft_params_t *params, uint8_t *x);

/**
 * Multiply and add polynomials in the frequency domain
 */
uint32_t* multiply_add_poly(const ntt_fft_params_t *params, uint32_t *s,
								   uint32_t *a, uint32_t *e);

/**
 * Multiply polynomials in the frequency domain and return to time domain
 */
uint32_t* multiply_ntt_inv_poly(const ntt_fft_params_t *params, uint32_t *s, uint32_t *b);

/**
 * Pack four 2-bit coefficents into one byte
 */
static inline void pack_rec(const ntt_fft_params_t *params, uint8_t *x, uint8_t *r)
{
	int i;

	for (i = 0; i < params->n; i += 4)
	{
		*x++ = r[i] | r[i+1] << 2 | r[i+2] << 4 | r[i+3] << 6;
	}
}

static inline uint8_t* unpack_rec(const ntt_fft_params_t *params, uint8_t *x)
{
	uint8_t *r;
	int i;

	r = (uint8_t*)malloc(params->n);

	for (i = 0; i < params->n; i += 4)
	{
		r[i]   = (*x)      & 0x03;
		r[i+1] = (*x >> 2) & 0x03;
		r[i+2] = (*x >> 4) & 0x03;
		r[i+3] = (*x >> 6) & 0x03;
		x++;
	}

	return r;
}

#endif /** NEWHOPE_POLY_H_ @}*/

