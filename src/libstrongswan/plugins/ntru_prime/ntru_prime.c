/**
 *  ntru_prime.c
 *  ntrup
 *
 *  Created by CJ Tjhai on 06/02/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntru_prime.h"
#include "ntru_prime_math_utils.h"
#include "ntru_prime_z_polynomial.h"
#include "ntru_prime_fisher_yates_shuffle.h"
#include "ntru_prime_pq_random.h"

#include <library.h>

#define NTRU_PRIME_P    547
#define NTRU_PRIME_Q    3001
#define NTRU_PRIME_T    62

typedef struct {
    z_poly *hx;         /* Public-key */
    z_poly *fx, *igx;   /* Private-key */
} ntru_prime_priv;

static const int kNTRUPrimeKEMKeysize = 32;

/* Private methods */
void digest(const uint8_t* buffer, size_t buffer_size, uint8_t *hash);
coeff_t *create_random_vector(int size, int weight, const coeff_t *c, int c_size);
uint8_t *serialise_public_key(const ntru_prime *ntrup, size_t *public_key_size);
ntru_prime *deserialise_public_key(const uint8_t *public_key, size_t public_key_size);
uint8_t *serialise_private_key(const ntru_prime *ntrup, size_t *private_key_size);
ntru_prime *deserialise_private_key(const uint8_t *private_key, size_t private_key_size);

ntru_prime* init_ntru_prime()
{
    int i;
    z_poly *mx3, *mxq;
    z_poly *fxq, *ifxq;
    z_poly *gx, *gxq;
    ntru_prime* ntrup;
    coeff_t *fq = NULL, *g = NULL;
    const coeff_t fq_coeff[] = { 0, 3, NTRU_PRIME_Q-3 };
    const coeff_t g_coeff[] = { 0, 1, 2 };
    ntru_prime_priv *priv = NULL;

    ntrup = (ntru_prime *)malloc(sizeof(ntru_prime));
    ntrup->p = NTRU_PRIME_P;
    ntrup->q = NTRU_PRIME_Q;
    ntrup->t = NTRU_PRIME_T;

    ntrup->priv = malloc(sizeof(ntru_prime_priv));
    if (!ntrup->priv)
        return NULL;
    priv = (ntru_prime_priv *)ntrup->priv;
    priv->fx = priv->hx = priv->igx = NULL;

    mx3 = init_poly(ntrup->p+1, 3);
    mx3->degree = ntrup->p;
    mx3->coeff[0] = mx3->coeff[1] = 2; mx3->coeff[ntrup->p] = 1;

    gx = init_poly(ntrup->p, 3);

    /* Generate a random vector g */
    priv->igx = NULL;
    do {
        if (g)
            free(g);
        g = create_random_vector(ntrup->p, 0, g_coeff, 3);
		if (!g)
			return NULL;

        gx->degree = ntrup->p-1;
        while (!g[gx->degree]) --gx->degree;
        memcpy(gx->coeff, g, ntrup->p*sizeof(coeff_t));

        if (priv->igx)
            free_poly(priv->igx);
        priv->igx = inverse_polynomial(gx, mx3);
    } while (!priv->igx);

    mxq = init_poly(ntrup->p+1, ntrup->q);
    mxq->degree = ntrup->p;
    mxq->coeff[0] = mxq->coeff[1] = ntrup->q-1; mxq->coeff[ntrup->p] = 1;

    fxq = init_poly(ntrup->p, ntrup->q);

    /* Generate a random vector f */
    fq = create_random_vector(ntrup->p, 2*ntrup->t, fq_coeff, 3);

    fxq->degree = ntrup->p-1;
    while (!fq[fxq->degree]) --fxq->degree;
    memcpy(fxq->coeff, fq, ntrup->p*sizeof(coeff_t));

    priv->fx = clone_poly(fxq);

    ifxq = inverse_polynomial(fxq, mxq);
    if (!ifxq)
        return NULL;

    /* We need g_q(x), i.e. g(x) mod q */
    gxq = clone_poly(gx);
    gxq->mod = ntrup->q;
    for (i=0; i<=gxq->degree; i++) {
        if (gxq->coeff[i] == 2)
            gxq->coeff[i] = ntrup->q-1;
    }

    priv->hx = multiply_poly(gxq, ifxq);
    modulo_reduce_poly(mxq, priv->hx);

    /* Public key: h(x) in Rq */
    ntrup->public_key = serialise_public_key(ntrup, &(ntrup->public_key_size));
    /* Private key: f in R and g^-1 in R3 */
    ntrup->private_key = serialise_private_key(ntrup, &(ntrup->private_key_size));

    zero_poly(mx3); free_poly(mx3);
    zero_poly(mxq); free_poly(mxq);
    zero_poly(fxq); free_poly(fxq);
    zero_poly(ifxq); free_poly(ifxq);
    zero_poly(gx); free_poly(gx);
    zero_poly(gxq); free_poly(gxq);
    memset(g, 0, ntrup->p*sizeof(coeff_t));
    free(g);
    memset(fq, 0, ntrup->p*sizeof(coeff_t));
    free(fq);

    return ntrup;
}

void free_ntru_prime(ntru_prime *ntrup)
{
    ntru_prime_priv *priv;

    if (ntrup) {
        if (ntrup->priv) {
            priv = (ntru_prime_priv *)ntrup->priv;
            if (priv->fx) {
                zero_poly(priv->fx);
                free_poly(priv->fx);
                priv->fx = NULL;
            }
            if (priv->igx) {
                zero_poly(priv->igx);
                free_poly(priv->igx);
                priv->igx = NULL;
            }
            if (priv->hx) {
                zero_poly(priv->hx);
                free_poly(priv->hx);
                priv->hx = NULL;
            }
            free(ntrup->priv);
            ntrup->priv = NULL;
        }
        if (ntrup->private_key) {
            memset(ntrup->private_key, 0, ntrup->private_key_size);
            free(ntrup->private_key);
            ntrup->private_key = NULL;
        }
        if (ntrup->public_key) {
            memset(ntrup->public_key, 0, ntrup->public_key_size);
            free(ntrup->public_key);
            ntrup->public_key = NULL;
        }
        free(ntrup);
    }
}

int ntru_prime_kem_key_size()
{
    return kNTRUPrimeKEMKeysize;
}

int ntru_prime_ciphertext_size(const ntru_prime *ntrup)
{
    /**
     * Q is 3001 and each coefficient of c up to this point
     * is not larger than Q/3, i.e. < 1001. Therefore, any
     * three coefficients of c will consume 30 bits, i.e.
     * they can be packed into 4 bytes
     **/
    return kNTRUPrimeKEMKeysize + (4*(ntrup->p + 2)/3);
}

int ntru_prime_ciphertext_size_from_public_key(const uint8_t *public_key,
                                               size_t public_key_size)
{
    int size = 0;
    ntru_prime *ntrup = NULL;

    ntrup = deserialise_public_key(public_key, public_key_size);
    if (ntrup) {
        size = ntru_prime_ciphertext_size(ntrup);
        
        free_ntru_prime(ntrup);
    }

    return size;
}

int ntru_prime_encapsulate(const uint8_t *public_key,
                           size_t public_key_size,
                           uint8_t *ciphertext,
                           uint8_t *key)
{
    register int i, j;
    size_t r_buf_size, c_buf_size;
    uint8_t *c_buf, *c_buf_ptr;
    uint8_t hash[ 64 ];
    uint8_t *r_buf, *r_buf_ptr;
    uint32_t c_packed_value;
    coeff_t q2, q6, i3, *c;
    z_poly *mx, *rx, *hrx;
    ntru_prime *ntrup = NULL;
    ntru_prime_priv *priv = NULL;
    const coeff_t c_coeff[] = { 0, 1, -1 };
    coeff_t *r;

    ntrup = deserialise_public_key(public_key, public_key_size);
    if (!ntrup)
        return 0;
    if (ntrup->priv)
        priv = (ntru_prime_priv *)ntrup->priv;

    /* Generate a uniform random t-small element */
    r = create_random_vector(ntrup->p, 2*ntrup->t, c_coeff, sizeof(c_coeff)/sizeof(coeff_t));

    rx = init_poly(ntrup->p, ntrup->q);
    memcpy(rx->coeff, r, ntrup->p*sizeof(coeff_t));
    rx->degree = ntrup->p;

    while (!rx->coeff[rx->degree]) --rx->degree;

    mx = init_poly(ntrup->p+1, ntrup->q);
    mx->degree = ntrup->p;
    mx->coeff[0] = mx->coeff[1] = ntrup->q - 1; mx->coeff[mx->degree] = 1;

    hrx = multiply_poly(priv->hx, rx);
    modulo_reduce_poly(mx, hrx);

    q2 = ntrup->q >> 1;
    c = (coeff_t *)calloc(ntrup->p+1, sizeof(coeff_t));
    for (i=0; i<=hrx->degree; i++) {
        c[i] = hrx->coeff[i];
        if (c[i] >= q2)
            c[i] -= ntrup->q;
        c[i] -= (3 * lrint((double)c[i] / 3.0));
        c[i] = hrx->coeff[i] - c[i];
        if (c[i] < 0)
            c[i] += ntrup->q;
    }

    r_buf_size = ((2*ntrup->p) + 7) / 8;
    r_buf = (uint8_t *)calloc(r_buf_size, sizeof(uint8_t));
    if (!r_buf)
        return 0;
    r_buf_ptr = r_buf;
    for (i=0; i<ntrup->p/4; i++,r_buf_ptr++) {
        j = (i << 2);
        *r_buf_ptr = (rx->coeff[j] + 1) + ((rx->coeff[j+1] + 1) << 2) +
            ((rx->coeff[j+2] + 1) << 4) + ((rx->coeff[j+3] + 1) << 6);
    }
    for (j=(4*i); j<ntrup->p; j++) {
        *r_buf_ptr += ((rx->coeff[j] + 1) << 2*(j-(4*i)));
    }

    digest(r_buf, r_buf_size, hash);

    memcpy(key, &hash[32], ntru_prime_kem_key_size());

    /**
     * Note: The polynomial c(x) is obtained as follows
     *   c(x) = e(x) + hr(x)
     * where e(x) is derived from hr(x). For the sake or argument, let
     *   c(x) = c_0 + c_1x + c2x^2 + ... + cnx^n
     * where degree(c(x)) = n. In order to encode/compress the coefficients
     * of c(x), they need to be put in vector form and padded with some zeros
     * so that the total length of the vector is p+1, i.e.
     *   c = [c_0, c_1, ..., c_n || 0..0 ]
     * The extra zeros are crucial here, they cannot be ignored! This is the
     * reason why we use:
     *     for (i=0; i<=ntrup->p; i++)
     * instead of
     *     for (i=0; i<=hrx->degree; i++)
     * in the loop below.
     **/
    q6 = (ntrup->q + 5) / 6;
    i3 = (coeff_t)modulo_inverse_prime(3, ntrup->q);
    for (i=0; i<ntrup->p; i++) {
        c[i] *= i3;
        c[i] -= (c[i]/ntrup->q)*ntrup->q;
        if (c[i] >= q2)
            c[i] -= ntrup->q;
        c[i] += q6;
    }

    /**
     * Q is 3001 and each coefficient of c up to this point
     * is not larger than Q/3, i.e. < 1001. Therefore, any
     * three coefficients of c will consume 30 bits, i.e.
     * they can be packed into 4 bytes
     **/
    c_buf_size = 4 * ((ntrup->p + 2)/3);
    c_buf = (uint8_t *)calloc(c_buf_size, sizeof(uint8_t));
    c_buf_ptr = c_buf;
    for (i=0,j=0; i<(ntrup->p+2)/3; i++,j+=3) {
        c_packed_value = c[j] + c[j+1]*(1 << 10) + c[j+2]*(1 << 20);
        memcpy(c_buf_ptr, &c_packed_value, 4);
        c_buf_ptr += 4;
    }

    memcpy(ciphertext, hash, kNTRUPrimeKEMKeysize);
    memcpy(&ciphertext[kNTRUPrimeKEMKeysize], c_buf, c_buf_size);

    memset(hash, 0, 2*kNTRUPrimeKEMKeysize);
    memset(c_buf, 0, r_buf_size);
    memset(r_buf, 0, r_buf_size);
    memset(c, 0, ntrup->p);
    memset(r, 0, ntrup->p*sizeof(coeff_t));

    free_ntru_prime(ntrup);
    zero_poly(hrx); free_poly(hrx);
    zero_poly(rx); free_poly(rx);
    free_poly(mx);
    free(r);
    free(c);
    free(r_buf);
    free(c_buf);

    return 1;
}

int ntru_prime_decapsulate(const uint8_t *private_key,
                           size_t private_key_size,
                           const uint8_t *ciphertext,
                           uint8_t *key)
{
    int i, j;
    coeff_t q2, q6;
    uint32_t v = 0;
    const uint8_t *buf_ptr = NULL;
    uint8_t *key_confirmation;
    uint8_t hash[ 2*kNTRUPrimeKEMKeysize ];
    uint8_t *r_buf, *r_buf_ptr;
    size_t r_buf_size = 0;
    int32_t *m, status = 0, weight_rx = 0;
    z_poly *cx, *mx, *rx;
    z_poly *hrx, *ccx, *fcx;
    ntru_prime *ntrup = NULL;
    ntru_prime_priv *priv = NULL;

    ntrup = deserialise_private_key(private_key, private_key_size);
    if (!ntrup)
        return 0;
    if (ntrup && ntrup->priv)
        priv = (ntru_prime_priv *)ntrup->priv;

    cx = init_poly(ntrup->p, ntrup->q);
    if (!cx)
        return 0;

    buf_ptr = ciphertext;

    /* Extract key confirmation from ciphertext */
    key_confirmation = (uint8_t *)calloc(kNTRUPrimeKEMKeysize, sizeof(uint8_t));
    if (!key_confirmation)
        return 0;
    memcpy(key_confirmation, buf_ptr, kNTRUPrimeKEMKeysize);
    buf_ptr += kNTRUPrimeKEMKeysize;

    /* Reconstruct c from the rest of the ciphertext */
    q2 = ntrup->q >> 1;
    q6 = (ntrup->q + 5) / 6;
    for (i=0,j=0; i<ntru_prime_ciphertext_size(ntrup)-kNTRUPrimeKEMKeysize-4; i+=4, j+=3) {
        v = 0;
        memcpy(&v, buf_ptr, 4);
        buf_ptr += 4;

        cx->coeff[j]   = ((v & ((1 << 10)-1)) - q6);
        if (cx->coeff[j] < q2)
            cx->coeff[j] += ntrup->q;
        cx->coeff[j+1] = (((v >> 10) & ((1 << 10)-1)) - q6);
        if (cx->coeff[j+1] < q2)
            cx->coeff[j+1] += ntrup->q;
        cx->coeff[j+2] = (((v >> 20) & ((1 << 10)-1)) - q6);
        if (cx->coeff[j+2] < q2)
            cx->coeff[j+2] += ntrup->q;

        cx->coeff[j] *= 3;
        while (cx->coeff[j]   >= ntrup->q) cx->coeff[j]   -= ntrup->q;
        cx->coeff[j+1] *= 3;
        while (cx->coeff[j+1] >= ntrup->q) cx->coeff[j+1] -= ntrup->q;
        cx->coeff[j+2] *= 3;
        while (cx->coeff[j+2] >= ntrup->q) cx->coeff[j+2] -= ntrup->q;
    }
    v = 0;
    memcpy(&v, buf_ptr, 3);
    if (j < cx->size) {
        cx->coeff[j]   = ((v & ((1 << 10)-1)) - q6);
        if (cx->coeff[j] < q2)
            cx->coeff[j] += ntrup->q;
        cx->coeff[j] *= 3;
        while (cx->coeff[j] >= ntrup->q) cx->coeff[j] -= ntrup->q;
    }
    if (j+1 < cx->size) {
        cx->coeff[j+1] = (((v >> 10) & ((1 << 10)-1)) - q6);
        if (cx->coeff[j+1] < q2)
            cx->coeff[j+1] += ntrup->q;
        cx->coeff[j+1] *= 3;
        while (cx->coeff[j+1] >= ntrup->q) cx->coeff[j+1] -= ntrup->q;
    }
    if (j+2 < cx->size) {
        cx->coeff[j+2] = (((v >> 20) & ((1 << 10)-1)) - q6);
        if (cx->coeff[j+2] < q2)
            cx->coeff[j+2] += ntrup->q;
        cx->coeff[j+2] *= 3;
        while (cx->coeff[j+2] >= ntrup->q) cx->coeff[j+2] -= ntrup->q;
    }
    cx->degree = cx->size-1;
    while (!cx->coeff[cx->degree]) --cx->degree;

    /* f_q(x) * c(x) modulo q */
    mx = init_poly(ntrup->p+1, ntrup->q);
    if (!mx)
        return 0;
    q2 = ntrup->q >> 1;
    mx->degree = ntrup->p;
    mx->coeff[0] = mx->coeff[1] = ntrup->q-1; mx->coeff[ntrup->p] = 1;
    fcx = multiply_poly(priv->fx, cx);
    modulo_reduce_poly(mx, fcx);

    for (i=0; i<=fcx->degree; i++) {
        if (fcx->coeff[i] >= q2)
            fcx->coeff[i] -= ntrup->q;
        fcx->coeff[i] = int_mod(fcx->coeff[i], 3);
    }
    fcx->mod = 3;

    /* R3[ g(x)^{-1} ] * R3[ f_q(x) * c(x) ] */
    mx->mod = 3;
    mx->coeff[0] = mx->coeff[1] = 2; mx->coeff[ntrup->p] = 1;
    rx = multiply_poly(priv->igx, fcx);
    modulo_reduce_poly(mx, rx);

    for (i=0; i<=rx->degree; i++)
        rx->coeff[i] -= (3 * lrint((double)rx->coeff[i] / 3.0));

    r_buf_size = ((2*ntrup->p) + 7) / 8;
    r_buf = (uint8_t *)calloc(r_buf_size, sizeof(uint8_t));
    if (!r_buf)
        return 0;

    r_buf_ptr = r_buf;
    for (i=0; i<ntrup->p/4; i++,r_buf_ptr++) {
        j = (i << 2);
        *r_buf_ptr = (rx->coeff[j] + 1) + ((rx->coeff[j+1] + 1) << 2) +
        ((rx->coeff[j+2] + 1) << 4) + ((rx->coeff[j+3] + 1) << 6);
    }
    for (j=(4*i); j<ntrup->p; j++) {
        *r_buf_ptr += ((rx->coeff[j] + 1) << 2*(j-(4*i)));
    }

    digest(r_buf, r_buf_size, hash);

    /* h * Rq(r) */
    mx->mod = ntrup->q;
    mx->coeff[0] = mx->coeff[1] = ntrup->q-1; mx->coeff[ntrup->p] = 1;
    coeff_mod_poly(rx, ntrup->q);
    hrx = multiply_poly(priv->hx, rx);
    modulo_reduce_poly(mx, hrx);

    q2 = ntrup->q >> 1;
    ccx = init_poly(ntrup->p, ntrup->q);
    m = (int32_t *)calloc(hrx->degree+1, sizeof(int32_t));
    for (i=0; i<=hrx->degree; i++) {
        if (hrx->coeff[i] >= q2)
            hrx->coeff[i] -= ntrup->q;
        ccx->coeff[i] = hrx->coeff[i];
        m[i] = (3 * (int32_t)lrint((double)hrx->coeff[i] / 3.0)) - hrx->coeff[i];
        ccx->coeff[i] += m[i];
        if (ccx->coeff[i] < 0)
            ccx->coeff[i] += ntrup->q;
    }
    ccx->degree = ccx->size-1;
    while (!ccx->coeff[ccx->degree]) --ccx->degree;

    weight_rx = 0; i = rx->degree+1; do {
        --i;
        weight_rx += (rx->coeff[i] != 0);
    } while (i);

    status = (weight_rx == 2*ntrup->t) && is_poly_equal(cx, ccx) &&
        !memcmp(hash, key_confirmation, kNTRUPrimeKEMKeysize);

    memcpy(key, &hash[ kNTRUPrimeKEMKeysize ], kNTRUPrimeKEMKeysize);

    memset(key_confirmation, 0, kNTRUPrimeKEMKeysize);
    memset(r_buf, 0, r_buf_size);
    memset(hash, 0, 2*kNTRUPrimeKEMKeysize);
    memset(m, 0, (hrx->degree+1)*sizeof(int32_t));

    free(m);
    free(r_buf);
    zero_poly(ccx); free_poly(ccx);
    zero_poly(hrx); free_poly(hrx);
    zero_poly(rx); free_poly(rx);
    zero_poly(cx); free_poly(cx);
    zero_poly(mx); free_poly(mx);
    zero_poly(fcx); free_poly(fcx);
    free_ntru_prime(ntrup);
    free(key_confirmation);

    return status;
}

#pragma mark - Private methods

void digest(const uint8_t* buffer, size_t buffer_size, uint8_t *hash)
{
	chunk_t data = chunk_empty;
	hasher_t *hasher = NULL;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA512);
	if (hasher) {
		data = chunk_create((uint8_t*)buffer, buffer_size);
		hasher->get_hash(hasher, data, hash);
		hasher->destroy(hasher);
	}
}

coeff_t *create_random_vector(int size, int weight, const coeff_t *c, int c_size)
{
    int index;
    uint32_t rnd, *buffer;
    coeff_t *c_buf = NULL;

    c_buf = (coeff_t *)calloc(size, sizeof(coeff_t));
    if (!c_buf)
        return NULL;

    rnd = 0;
    index = 0;
    if (weight <= 0) {   /* No restrictions on weight */
        for (index=0; index<size; index++) {
            pq_rand_uint32_at_most(c_size-1, &rnd);
            c_buf[index] = c[rnd];
        }
    } else {            /* There is weight restrictions */
        buffer = (uint32_t *)malloc(size * sizeof(uint32_t));
        if (!buffer)
            return NULL;
        for (index=0; index<size; index++)
            buffer[index] = index;

        fisher_yates_shuffle(buffer, size);

        for (index=0; index<weight; index++) {
            /* Make sure that it's a non-zero element */
            do {
                pq_rand_uint32_at_most(c_size-1, &rnd);
            } while (!c[rnd]);
            c_buf[ buffer[index] ] = c[rnd];
        }
        free(buffer);
    }

    return c_buf;
}

uint8_t *encoded_coefficients(const coeff_t *c, size_t c_size, size_t *encoded_size)
{
    int i;
    uint32_t v;
    uint8_t *ec_ptr, *ec = NULL;
    const int m = 2, n = 3, b = 4096;
    size_t ec_size, size = (c_size + (m-1)) / m;

    *encoded_size = 0;
    ec_size = n * size;
    ec = (uint8_t *)calloc(ec_size, sizeof(uint8_t));
    if (ec == NULL)
        return NULL;
    ec_ptr = ec;
    for (i=0; i<m*size; i+=m) {
        v = (c[i] + (b * c[i+1]));
        memcpy(ec_ptr, &v, n);
        ec_ptr += n;
    }

    if (encoded_size)
        *encoded_size = ec_size;

    return ec;
}

coeff_t *decoded_coefficients(const uint8_t *ec, size_t encoded_size, size_t *c_size)
{
    int i, j;
    uint32_t v;
    size_t size = 0;
    coeff_t *c = NULL;
    const uint8_t *ec_ptr = ec;
    const int m = 2, n = 3, b = 12;

    size = m * encoded_size / n;
    c = (coeff_t *)calloc(size, sizeof(coeff_t));
    if (c == NULL)
        return NULL;
    for (i=0,j=0; i<encoded_size; i+=n,j+=m) {
        v = 0;
        memcpy(&v, ec_ptr, n);
        ec_ptr += n;

        c[j]   = (v & ((1<<b)-1));
        c[j+1] = (v >> b) & ((1<<b)-1);
    }

    if (c_size)
        *c_size = size;

    return c;
}

uint8_t *serialise_public_key(const ntru_prime *ntrup, size_t *public_key_size)
{
    size_t ec_size = 0;
    ntru_prime_priv *priv = NULL;
    uint8_t *ptr, *pk_buffer = NULL;
    uint8_t *hx_coeff = NULL;

    if (ntrup && ntrup->priv)
        priv = (ntru_prime_priv *)ntrup->priv;

    /**
     * Each coefficient is less than NTRU_PRIME_Q, therefore we could take
     * a pair of coefficients and encode them into 3 bytes
     **/
    hx_coeff = encoded_coefficients(priv->hx->coeff, priv->hx->size, &ec_size);
    if (!hx_coeff)
        return NULL;

    *public_key_size = (3*sizeof(uint32_t)) + (ec_size);
    pk_buffer = (uint8_t *)malloc(*public_key_size);
    if (!pk_buffer) {
        *public_key_size = 0;
        return NULL;
    }

    ptr = pk_buffer;
    memcpy(ptr, &(ntrup->p), sizeof(ntrup->p)); ptr += sizeof(ntrup->p);
    memcpy(ptr, &(ntrup->q), sizeof(ntrup->q)); ptr += sizeof(ntrup->q);
    memcpy(ptr, &(ntrup->t), sizeof(ntrup->t)); ptr += sizeof(ntrup->t);
    memcpy(ptr, hx_coeff, ec_size);

    free(hx_coeff);

    return pk_buffer;
}

ntru_prime *deserialise_public_key(const uint8_t *public_key, size_t public_key_size)
{
    const int m = 2, n = 3;
    ntru_prime *ntrup = NULL;
    ntru_prime_priv *priv = NULL;
    const uint8_t *ptr = public_key;
    size_t coeff_size, size;

    ntrup = (ntru_prime *)malloc(sizeof(ntru_prime));
    if (!ntrup)
        return NULL;
    ntrup->priv = malloc(sizeof(ntru_prime_priv));
    if (!ntrup->priv)
        return NULL;
    priv = (ntru_prime_priv *)ntrup->priv;
    ntrup->private_key = ntrup->public_key = NULL;
    priv->fx = priv->hx = priv->igx = NULL;

    memcpy(&(ntrup->p), ptr, sizeof(ntrup->p)); ptr += sizeof(ntrup->p);
    memcpy(&(ntrup->q), ptr, sizeof(ntrup->q)); ptr += sizeof(ntrup->q);
    memcpy(&(ntrup->t), ptr, sizeof(ntrup->t)); ptr += sizeof(ntrup->t);

    size = ((ntrup->p + (m-1)) / m) * n;

    priv->hx = init_poly(ntrup->p, ntrup->q);
    if (!priv->hx)
        return NULL;
    free(priv->hx->coeff);
    priv->hx->coeff = decoded_coefficients(ptr, size, &coeff_size);

    priv->hx->degree = ntrup->p;
    while (!priv->hx->coeff[ priv->hx->degree ]) --priv->hx->degree;

    ntrup->public_key_size = public_key_size;
    ntrup->public_key = (uint8_t *)malloc(public_key_size);
    if (!ntrup->public_key)
        return NULL;
    memcpy(ntrup->public_key, public_key, public_key_size);

    return ntrup;
}

uint8_t *serialise_private_key(const ntru_prime *ntrup, size_t *private_key_size)
{
    uint8_t *ptr, *sk_buffer = NULL;
    ntru_prime_priv *priv = NULL;
    uint8_t *ec_fx_buf, *ec_igx_buf, *ec_hx_buf;
    size_t ec_fx_size, ec_igx_size, ec_hx_size;

    if (ntrup && ntrup->priv)
        priv = (ntru_prime_priv *)ntrup->priv;

    if (!priv || !priv->fx || !priv->igx || !priv->hx)
        return NULL;

    ec_fx_buf = encoded_coefficients(priv->fx->coeff, priv->fx->size, &ec_fx_size);
    ec_igx_buf = encoded_coefficients(priv->igx->coeff, priv->igx->size, &ec_igx_size);
    ec_hx_buf = encoded_coefficients(priv->hx->coeff, priv->hx->size, &ec_hx_size);

    if (!ec_fx_buf || !ec_igx_buf || !ec_hx_buf)
        return NULL;

    /**
     * Note that both priv->fx->size and priv->hx->size equal to ntrup->p,
     * but priv->igx->size equals to ntrup->p+1
     **/
    *private_key_size = (3*sizeof(uint32_t)) + (ec_fx_size + ec_igx_size + ec_hx_size);
    sk_buffer = (uint8_t *)malloc(*private_key_size);
    if (!sk_buffer) {
        *private_key_size = 0;
        return NULL;
    }
    ptr = sk_buffer;
    memcpy(ptr, &(ntrup->p), sizeof(ntrup->p)); ptr += sizeof(ntrup->p);
    memcpy(ptr, &(ntrup->q), sizeof(ntrup->q)); ptr += sizeof(ntrup->q);
    memcpy(ptr, &(ntrup->t), sizeof(ntrup->t)); ptr += sizeof(ntrup->t);
    memcpy(ptr, ec_fx_buf, ec_fx_size); ptr += ec_fx_size;
    memcpy(ptr, ec_igx_buf, ec_igx_size); ptr += ec_igx_size;
    memcpy(ptr, ec_hx_buf, ec_hx_size);

    memset(ec_fx_buf, 0, ec_fx_size);
    memset(ec_igx_buf, 0, ec_igx_size);
    memset(ec_hx_buf, 0, ec_hx_size);
    free(ec_fx_buf);
    free(ec_igx_buf);
    free(ec_hx_buf);

    return sk_buffer;
}

ntru_prime *deserialise_private_key(const uint8_t *private_key, size_t private_key_size)
{
    const int m = 2, n = 3;
    size_t size, igx_size;
    ntru_prime *ntrup = NULL;
    ntru_prime_priv *priv = NULL;
    const uint8_t *ptr = private_key;

    ntrup = (ntru_prime *)malloc(sizeof(ntru_prime));
    if (!ntrup)
        return NULL;
    ntrup->priv = malloc(sizeof(ntru_prime_priv));
    if (!ntrup->priv)
        return NULL;
    priv = (ntru_prime_priv *)ntrup->priv;
    ntrup->private_key = ntrup->public_key = NULL;
    priv->fx = priv->hx = priv->igx = NULL;

    memcpy(&(ntrup->p), ptr, sizeof(ntrup->p)); ptr += sizeof(ntrup->p);
    memcpy(&(ntrup->q), ptr, sizeof(ntrup->q)); ptr += sizeof(ntrup->q);
    memcpy(&(ntrup->t), ptr, sizeof(ntrup->t)); ptr += sizeof(ntrup->t);

    size = ((ntrup->p + (m-1)) / m) * n;
    igx_size = ((ntrup->p+1 + (m-1)) / m) * n;

    /* f(x) */
    priv->fx = init_poly(ntrup->p, ntrup->q);
    if (!priv->fx)
        return NULL;
    free(priv->fx->coeff);
    priv->fx->coeff = decoded_coefficients(ptr, size, NULL);
    if (!priv->fx->coeff)
        return NULL;
    ptr += size;

    priv->fx->degree = ntrup->p;
    while (!priv->fx->coeff[ priv->fx->degree ]) --priv->fx->degree;

    /* g(x)^{-1} */
    priv->igx = init_poly(ntrup->p, 3);
    if (!priv->igx)
        return NULL;
    free(priv->igx->coeff);
    priv->igx->coeff = decoded_coefficients(ptr, igx_size, NULL);
    if (!priv->igx->coeff)
        return NULL;
    ptr += igx_size;

    priv->igx->degree = ntrup->p;
    while (!priv->igx->coeff[ priv->igx->degree ]) --priv->igx->degree;

    /* h(x) */
    priv->hx = init_poly(ntrup->p, ntrup->q);
    if (!priv->hx)
        return NULL;
    free(priv->hx->coeff);
    priv->hx->coeff = decoded_coefficients(ptr, size, NULL);
    if (!priv->hx->coeff)
        return NULL;

    priv->hx->degree = ntrup->p;
    while (!priv->hx->coeff[ priv->hx->degree ]) --priv->hx->degree;

    ntrup->private_key_size = private_key_size;
    ntrup->private_key = (uint8_t *)malloc(private_key_size);
    if (!ntrup->private_key)
        return NULL;
    memcpy(ntrup->private_key, private_key, private_key_size);

    return ntrup;
}
