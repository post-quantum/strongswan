/*
 * Copyright (C) 2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Based on public domain code by Erdem Alkim, Léo Ducas, Thomas Pöppelmann,
 * and Peter Schwabe.
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

#include "newhope_poly.h"

#include <ntt_fft.h>
#include <ntt_fft_reduce.h>
#include <utils/debug.h>


/**
 * Derive 14-bit coefficients of polynomial a from 256 bit random seed
 * using the SHAKE128 extended output function
 */
uint32_t* derive_a_poly(const ntt_fft_params_t *params, chunk_t seed)
{
	uint32_t *a;
	uint8_t x[2];
	int i = 0;
	xof_t *xof;

	xof = lib->crypto->create_xof(lib->crypto, XOF_SHAKE_128);
	if (!xof)
	{
		DBG1(DBG_LIB, "could not instantiate SHAKE128 XOF");
		return NULL;
	}

	if (!xof->set_seed(xof, seed))
	{
		DBG1(DBG_LIB, "could not set seed of SHAKE128 XOF");
		xof->destroy(xof);
		return NULL;
	}

	/* allocate dynamic memory for polynomial a */
	a = (uint32_t*)malloc(params->n * sizeof(uint32_t));

	while (i < params->n)
	{
		if (!xof->get_bytes(xof, sizeof(x), x))
		{
			DBG1(DBG_LIB, "could not get bytes from SHAKE128 XOF");
			xof->destroy(xof);
			free(a);
			return NULL;
		}

		/*
		 * Treat x as a 16 bit unsigned little endian integer
		 * and truncate to 14 bits
		 */
		a[i] = uletoh16(x) & 0x3fff;

		if (a[i] < params->q)
		{
			i++;
		}
	}
	xof->destroy(xof);

	return a;
}

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
void pack_poly(const ntt_fft_params_t *params, uint8_t *x, uint32_t *p)
{
	int i;

	for (i = 0; i < params->n; i += 4)
	{
		*x++ = (p[i] & 0xff );
		*x++ = (p[i]   >>  8) | (p[i+1] << 6);
		*x++ = (p[i+1] >>  2);
		*x++ = (p[i+1] >> 10) | (p[i+2] << 4);
		*x++ = (p[i+2] >>  4);
		*x++ = (p[i+2] >> 12) | (p[i+3] << 2);
		*x++ = (p[i+3] >>  6);
	}
}

/**
 * Unpack seven consecutive bytes into four 14-bit coefficients
 */
uint32_t* unpack_poly(const ntt_fft_params_t *params, uint8_t *x)
{
	uint32_t *p;
	int i;

	p = (uint32_t*)malloc(params->n * sizeof(uint32_t));

	for (i = 0; i < params->n; i += 4)
	{
		p[i]   =  x[0]       | (((uint32_t)x[1] & 0x3f) <<  8);
		p[i+1] = (x[1] >> 6) | (((uint32_t)x[2]) <<  2)
							 | (((uint32_t)x[3] & 0x0f) << 10);
		p[i+2] = (x[3] >> 4) | (((uint32_t)x[4]) <<  4)
							 | (((uint32_t)x[5] & 0x03) << 12);
		p[i+3] = (x[5] >> 2) | (((uint32_t)x[6]) <<  6);
		x += 7;
	}
	for (i = 0; i < params->n; i++)
	{
		if (p[i] >= params->q)
		{
			DBG1(DBG_LIB, "polynomial coefficient must be smaller than %u",
						   params->q);
			free(p);
			return NULL;
		}
	}
	return p;
}

/**
 * Multiply and add polynomials in the frequency domain
 */
uint32_t* multiply_add_poly(const ntt_fft_params_t *params, uint32_t *s,
								   uint32_t *a, uint32_t *e)
{
	ntt_fft_t *fft;
	uint32_t *b, t;
	int i;

	/* transform s and h to frequency domain */
	fft = ntt_fft_create(params);
	fft->transform(fft, s, s, FALSE);
	fft->transform(fft, e, e, FALSE);
	fft->destroy(fft);

	b = (uint32_t*)malloc(params->n * sizeof(uint32_t));

	/* compute  b = a * s + e in the frequency domain */
	for (i = 0; i < params->n; i++)
	{
		/* convert a[i] to Montgomery domain */
		t = ntt_fft_mreduce(a[i] * params->r2, params);

		/* compute b[i] = a[i] * s[i] + e[i] in Montgomery domain */
		t = ntt_fft_mreduce(t * s[i], params) + e[i];

 		/* exit Montgomery domain before transmitting polynomial b */
		b[i] = ntt_fft_mreduce(t, params);
	}
	memwipe(e, params->n * sizeof(uint32_t));

	return b;
}

/**
 * Multiply polynomials in the frequency domain and return to time domain
 */
uint32_t* multiply_ntt_inv_poly(const ntt_fft_params_t *params, uint32_t *s, uint32_t *b)
{
	ntt_fft_t *fft;
	uint32_t *v, t;
	int i;

	v = (uint32_t*)malloc(params->n * sizeof(uint32_t));

	for (i = 0; i < params->n; i++)
	{
		/* convert b[i] to Montgomery domain */
		t = ntt_fft_mreduce(b[i] * params->r2, params);

		/* compute v[i] = b[i] * s[i] in Montgomery domain */
		v[i] = ntt_fft_mreduce(t * s[i], params);
	}

	/* transform v back to time domain */
	fft = ntt_fft_create(params);
	fft->transform(fft, v, v, TRUE);
	fft->destroy(fft);

	return v;
}


