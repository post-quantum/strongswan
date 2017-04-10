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

#ifdef QSKE

#include "newhope_qske.h"
#include "newhope_poly.h"
#include "newhope_noise.h"
#include "newhope_reconciliation.h"

#include <crypto/quantum_safe.h>
#include <utils/debug.h>

static const int seed_len =   32;  /* 256 bits */
static const int poly_len = 1792;  /* size of 1024 packed 14-bit coefficients */
static const int rec_len =   256;  /* size of 1024 packed  2-bit coefficients */

typedef struct private_newhope_qske_t private_newhope_qske_t;

/**
 * Private data of an newhope_qske_t object.
 */
struct private_newhope_qske_t {

	/**
	 * Public newhope_qske_t interface.
	 */
	newhope_qske_t public;

	/**
	 * FFT parameter set
	 */
	const ntt_fft_params_t *params;

	/**
	 * Secret noise polynomial s
	 */
	uint32_t *s;

	/**
	 * Output polynomial u = a * NTT(s') + NTT(e')
	 */
	uint32_t *u;

	/**
	 * Error reconciliation help bits
	 */
	uint8_t *r;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

};


#define METHOD_KE(name, ret, ...) METHOD(quantum_safe_t, name, ret, private_newhope_qske_t *this, ##__VA_ARGS__)

METHOD_KE(get_my_public_value, bool, chunk_t *value)
{
	uint16_t n, q;
	int i;

	/* Define some often-used constants */
	n = this->params->n;
	q = this->params->q;

	/* are we the initiator? */
	if (this->u == NULL)
	{
		rng_t *rng;
		uint32_t *a = NULL, *b = NULL, *e = NULL;
		uint8_t noise_seed_buf[seed_len];
		chunk_t noise_seed = { noise_seed_buf, seed_len};
		chunk_t a_seed;
		newhope_noise_t *noise = NULL;
		bool success = FALSE;

		/* allocate space for public output value */
		*value = chunk_alloc(poly_len + seed_len);
		a_seed = chunk_create(value->ptr + poly_len, seed_len);

		/* create polynomial a from 256 bit random seed */
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng)
		{
			DBG1(DBG_LIB, "could not instatiate random source");
			return FALSE;
		}
		if (!rng->get_bytes(rng, seed_len, a_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for polynomial a");
			goto end;
		}

		a = derive_a_poly(this->params, a_seed);
		if (a == NULL)
		{
			goto end;
		}

		/* generate random seed for the derivation of noise polynomials */
		if (!rng->get_bytes(rng, seed_len, noise_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for noise polynomials");
			goto end;
		}

		/* create noise polynomial generator */
		noise = newhope_noise_create(noise_seed);
		if (!noise)
		{
			goto end;
		}

		/* create noise polynomial s from seed with nonce = 0x00 */
		this->s = noise->get_binomial_words(noise, 0x00, n, q);
		if (this->s == NULL)
		{
			goto end;
		}

		/* create noise polynomial e from seed with nonce = 0x01 */
		e = noise->get_binomial_words(noise, 0x01, n, q);
		if (e == NULL)
		{
			goto end;
		}

		/* compute b = a * NTT(s) + NTT(e) */
		b = multiply_add_poly(this->params, this->s, a, e);

		/*DBG3(DBG_LIB, "   i  a[i]  b[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, a[i], b[i]);
		}*/

		/* pack coefficients of polynomial b */
		pack_poly(this->params, value->ptr, b);
		success = TRUE;

	end:
		rng->destroy(rng);
		DESTROY_IF(noise);
		free(a);
		free(b);
		free(e);

		if (!success)
		{
		chunk_free(value);
		}
		return success;
	}
	else
	{
		DBG3(DBG_LIB, "   i  u[i]  r[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, this->u[i], this->r[i]);
		}

		/* allocate space for public output value */
		*value = chunk_alloc(poly_len + rec_len);

		/* pack coefficients of polynomial u */
		pack_poly(this->params, value->ptr, this->u);

		/* pack coefficients of polynomial r */
		pack_rec(this->params, value->ptr + poly_len, this->r);

		return TRUE;
	}
}

METHOD_KE(get_shared_secret, bool, chunk_t *secret)
{
	if (this->shared_secret.len == 0)
	{
		*secret = chunk_empty;
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);

	return TRUE;
}

METHOD_KE(set_other_public_value, bool, chunk_t value)
{
	newhope_reconciliation_t * rec;
	uint16_t n, q;
	int i;

	/* Define some often-used constants */
	n = this->params->n;
	q = this->params->q;

	/* are we the responder? */
	if (this->s == NULL)
	{
		uint32_t *a = NULL, *b = NULL, *e1 = NULL, *e2 = NULL, *v = NULL, t;
		uint8_t *rbits = NULL;
		uint8_t noise_seed_buf[seed_len];
		chunk_t noise_seed = { noise_seed_buf, seed_len };
		chunk_t a_seed;
		newhope_noise_t *noise = NULL;
		rng_t *rng = NULL;
		bool success = FALSE;

		if (value.len != poly_len + seed_len)
		{
			DBG1(DBG_LIB, "received %N KE payload of incorrect size",
						   diffie_hellman_group_names, NH_128_BIT);
			return FALSE;
		}
		a_seed = chunk_create(value.ptr + poly_len, seed_len);

		a = derive_a_poly(this->params, a_seed);
		if (a == NULL)
		{
			return FALSE;
		}

		b = unpack_poly(this->params, value.ptr);
		if (b == NULL)
		{
			goto end;
		}

		/* debug output of polynomials a and b */
		/*DBG3(DBG_LIB, "   i  a[i]  b[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, a[i], b[i]);
		}*/

		/* generate random seed for the derivation of noise polynomials */
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng)
		{
			DBG1(DBG_LIB, "could not instatiate random source");
			goto end;
		}
		if (!rng->get_bytes(rng, seed_len, noise_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for noise polynomials");
			goto end;
		}

		/* create noise polynomial generator */
		noise = newhope_noise_create(noise_seed);
		if (!noise)
		{
			goto end;
		}

		/* create noise polynomial s' from seed with nonce = 0x00 */
		this->s = noise->get_binomial_words(noise, 0x00, n, q);
		if (this->s == NULL)
		{
			goto end;
		}

		/* create noise polynomial e' from seed with nonce = 0x01 */
		e1 = noise->get_binomial_words(noise, 0x01, n, q);
		if (e1 == NULL)
		{
			goto end;
		}

		/* create noise polynomial e'' from seed with nonce = 0x02 */
		e2 = noise->get_binomial_words(noise, 0x02, n, q);
		if (e2 == NULL)
		{
			goto end;
		}

		/* compute u = a * NTT(s') + NTT(e') */
		this->u = multiply_add_poly(this->params, this->s, a, e1);

		/* compute v = NTT_inv( b * NTT(s') ) */
		v = multiply_ntt_inv_poly(this->params, this->s, b);

		/* compute v = v + e'' */
		for (i = 0; i < n; i++)
		{
			t = v[i] + e2[i];
			v[i] = (t < q) ? t : t - q;
		}
		memwipe(e2, n * sizeof(uint32_t));

		/* create uniform noise bytes from seed with nonce = 0x02 */
		rbits = noise->get_uniform_bytes(noise, 0x03, n/(4*8));

		rec = newhope_reconciliation_create(n, q);
		this->r = rec->help_reconcile(rec, v, rbits);
		free(rbits);
		this->shared_secret = rec->reconcile(rec, v, this->r);
		rec->destroy(rec);

		DBG4(DBG_LIB, "key: %B", &this->shared_secret);
		success = TRUE;

	end:
		DESTROY_IF(rng);
		DESTROY_IF(noise);
		free(a);
		free(b);
		free(e1);
		free(e2);
		free(v);

		return success;
	}
	else
	{
		uint32_t *v;

		if (value.len != poly_len + rec_len)
		{
			DBG1(DBG_LIB, "received %N KE payload of incorrect size",
						   diffie_hellman_group_names, NH_128_BIT);
			return FALSE;
		}

		this->u = unpack_poly(this->params, value.ptr);
		if (this->u == NULL)
		{
			return FALSE;
		}

		this->r = unpack_rec(this->params, value.ptr + poly_len);
		if (this->r == NULL)
		{
			return FALSE;
		}

		DBG3(DBG_LIB, "   i  u[i]  r[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, this->u[i], this->r[i]);
		}

		/* compute v' = NTT_inv( u * NTT(s) ) */
		v = multiply_ntt_inv_poly(this->params, this->s, this->u);

		rec = newhope_reconciliation_create(n, q);
		this->shared_secret = rec->reconcile(rec, v, this->r);
		free(v);
		rec->destroy(rec);

		DBG4(DBG_LIB, "key: %B", &this->shared_secret);

		return TRUE;
	}
}


METHOD_KE(get_qs_group, quantum_safe_group_t)
{
	return QS_NH_128_BIT;
}

METHOD_KE(destroy, void)
{
	chunk_clear(&this->shared_secret);
	memwipe(this->s, this->params->n * sizeof(uint32_t));
	free(this->s);
	free(this->u);
	free(this->r);
	free(this);
}

/*
 * Described in header.
 */
newhope_qske_t *newhope_qske_create(quantum_safe_group_t group, chunk_t g, chunk_t p)
{
	private_newhope_qske_t *this;

	INIT(this,
		.public = {
			.qs = {
				.get_qs_group = _get_qs_group,
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.destroy = _destroy,
			},
		},
		.params = &ntt_fft_12289_1024,

	);

	return &this->public;
}

#endif
