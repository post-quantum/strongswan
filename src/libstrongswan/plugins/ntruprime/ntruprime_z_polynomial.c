/**
 *  polynomial.c
 *  ntrup
 *
 *  Created by CJ on 11/01/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntruprime_z_polynomial.h"
#include "ntruprime_pq_random.h"
#include "ntruprime_fisher_yates_shuffle.h"
#include "ntruprime_math_utils.h"

#define COPY_POLY(a, b)  (b)->degree = (a)->degree; (b)->mod = (a)->mod; \
                         memcpy((b)->coeff, (a)->coeff, (a)->size*sizeof(coeff_t))

z_poly* init_poly(int size, coeff_t mod)
{
    z_poly* px = (z_poly *)malloc(sizeof(z_poly));
    if (!px)
        return NULL;
    
    px->size = size;
    px->mod = mod;
    px->degree = -1; /* Indicate a zero polynomial */
    px->coeff = (coeff_t *)calloc(size, sizeof(coeff_t));
    if (!px->coeff)
        return NULL;
    
    return px;
}

void free_poly(z_poly* px)
{
    if (px) {
        if (px->coeff)
            free(px->coeff);
        px->coeff = NULL;
        px->degree = -1;
        px->size = 0;
        px->mod = 0;
        free(px);
    }
}

void update_poly_degree(z_poly *ax)
{
    ax->degree = ax->size-1;
    while ((ax->degree >= 0) && !ax->coeff[ax->degree])
        --ax->degree;
}

void zero_poly(z_poly* px)
{
    px->degree = -1;
    memset(px->coeff, 0, px->size*sizeof(coeff_t));
}

int is_zero_poly(const z_poly *px)
{
    return (px->degree <= 0) && (!px->coeff[0]);
}

z_poly* clone_poly(const z_poly *px)
{
    z_poly *qx = init_poly(px->size, px->mod);
    if (!qx)
        return NULL;
    
    memcpy(qx->coeff, px->coeff, px->size*sizeof(coeff_t));
    qx->degree = px->degree;
    
    return qx;
}

void normalise_poly(z_poly *px)
{
    int i;
    for (i=0; i<=px->degree; i++) {
        px->coeff[i] = int_mod(px->coeff[i], px->mod);
    }
}

z_poly *create_random_poly_with_constraints(int size,
                                            coeff_t mod,
                                            const coeff_t *c,
                                            size_t c_size,
                                            int weight,
                                            int degree)
{
    int i, sz;
    uint32_t rnd;
    z_poly *ax = NULL;
    uint32_t *buffer = NULL;
    
    if (degree >= size)
        return NULL;
    
    ax = init_poly(size, mod);
    if (!ax)
        return NULL;
    
    /* If there is no restriction on weight */
    if (weight <= 0) {
        /* Make sure that the first coefficient is non zero */
        do {
            pq_rand_uint32_at_most((uint32_t)c_size-1, &rnd);
            ax->coeff[0] = c[rnd];
        } while (!ax->coeff[0]);
        for (i=1; i<size; i++) {
            pq_rand_uint32_at_most(mod-1, &rnd);
            ax->coeff[i] = c[rnd];
            if (ax->coeff[i])
                ax->degree = i;
        }
        
        /* Any restriction on the degree of the polynomial */
        if (degree > 0) {
            if (ax->degree < degree) {
                /* Make sure that the last coefficient is non zero */
                do {
                    pq_rand_uint32_at_most((uint32_t)c_size-1, &rnd);
                    ax->coeff[degree] = c[rnd];
                } while (!ax->coeff[degree]);
            } else if (ax->degree > degree) {
                for (i=ax->degree; i>degree; i--) {
                    ax->coeff[i] = 0;
                }
            }
            ax->degree = degree;
        }
    }
    else {  /* There is a restriction weight */
        sz = size;
        if (degree > 0)
            sz = degree + 1;
        
        buffer = (uint32_t *)malloc(sz * sizeof(uint32_t));
        if (!buffer)
            return NULL;
        for (i=0; i<sz; i++)
            buffer[i] = i;
        
        fisher_yates_shuffle(buffer, sz);
        
        ax->degree = 0;
        for (i=0; i<weight; i++) {
            /* Make sure that it's a non-zero element */
            do {
                pq_rand_uint32_at_most((uint32_t)c_size-1, &rnd);
            } while (!c[rnd]);
            
            ax->coeff[ buffer[i] ] = c[rnd];
            if (ax->degree < buffer[i])
                ax->degree = buffer[i];
        }
        
        if (degree > 0 && ax->degree != degree) {
            ax->coeff[ ax->degree ] = 0;
            do {
                pq_rand_uint32_at_most((uint32_t)c_size-1, &rnd);
            } while (!c[rnd]);
            ax->coeff[ degree ] = c[rnd];
            ax->degree = degree;
        }
        
        free(buffer);
    }
    
    return ax;
}

void coeff_mod_poly(z_poly *ax, coeff_t p)
{
    register int i;
    register coeff_t a;
    for (i=0; i<=ax->degree; i++) {
        a = ax->coeff[i];
        if (a) {
            a %= p;
            if (a < 0)
                a += p;
            ax->coeff[i] = a;
        }
    }
    ax->mod = p;
}

z_poly *multiply_poly(const z_poly *ax, const z_poly *bx)
{
    int i, j;
    z_poly *cx = init_poly(ax->size + bx->size, ax->mod);
    
    for (i=0; i<=ax->degree; i++) {
        /* Make sure that coefficients are non-zero */
        if (ax->coeff[i]) {
            for (j=0; j<=bx->degree; j++) {
                if (bx->coeff[j]) {
                    cx->coeff[i + j] += (ax->coeff[i] * bx->coeff[j]);
                    cx->coeff[i + j]  = int_mod(cx->coeff[i + j], cx->mod);
                }
            }
        }
    }
    update_poly_degree(cx);

    return cx;
}

int divide_poly(const z_poly *ax, const z_poly *bx, z_poly **qx, z_poly **rx)
{
    coeff_t u, v;
    int i, size, offset;

    size = ax->size;
    if (size < bx->size)
        size = bx->size;
    *rx = clone_poly(ax);
    *qx = init_poly(size, ax->mod);
    if (!(*rx) || !(*qx))
        return 0;
    
    u = (coeff_t)modulo_inverse_prime(bx->coeff[bx->degree], bx->mod);
    (*qx)->degree = (*rx)->degree - bx->degree;
    while ((*rx)->degree >= 0 && ((*rx)->degree >= bx->degree)) {
        offset = (*rx)->degree - bx->degree;
        v = int_mod(u * (*rx)->coeff[ (*rx)->degree ], (*rx)->mod);
        for (i=0; i<bx->degree; i++) {
            if (bx->coeff[i]) {
                (*rx)->coeff[offset + i] -= (v * bx->coeff[i]);
                (*rx)->coeff[offset + i] = int_mod((*rx)->coeff[offset + i], (*rx)->mod);
            }
        }
        (*rx)->coeff[ (*rx)->degree ] = 0;
        while (((*rx)->degree >= 0) && !(*rx)->coeff[(*rx)->degree])
            --(*rx)->degree;
        
        (*qx)->coeff[offset] += v;
        (*qx)->coeff[offset] = int_mod((*qx)->coeff[offset], (*qx)->mod);
    }
    while (((*qx)->degree >= 0) && !(*qx)->coeff[(*qx)->degree])
        --(*qx)->degree;
    
    return 1;
}

void modulo_reduce_poly(const z_poly *mx, z_poly *ax)
{
    coeff_t u, v;
    int i, offset;
    
    u = (coeff_t)modulo_inverse_prime(mx->coeff[mx->degree], mx->mod);
    while (ax->degree >= mx->degree) {
        offset = ax->degree - mx->degree;
        v = u * ax->coeff[ ax->degree ];
        for (i=0; i<mx->degree; i++) {
            if (mx->coeff[i]) {
                ax->coeff[offset + i] -= (v * mx->coeff[i]);
                ax->coeff[offset + i] = int_mod(ax->coeff[offset + i], ax->mod);
            }
        }
        ax->coeff[ ax->degree ] = 0;
        update_poly_degree(ax);
    }
    
    /* Note we don't free the unused space occupied by a(x) */
    if (ax->size > mx->size)
        ax->size = mx->size;
}

int gcd_poly(const z_poly* ax, const z_poly *bx, z_poly *gx)
{
    z_poly *sx, *tx;
    
    sx = clone_poly(ax);
    tx = clone_poly(bx);
    if (!sx || !tx)
        return 0;
    
    while (tx->degree >= 0) {
        /* g(x) = s(x) */
        COPY_POLY(sx, gx);
        
        /* g(x) = g(x) mod t(x) */
        modulo_reduce_poly(tx, gx);
        
        /* s(x) = t(x) */
        COPY_POLY(tx, sx);
        
        /* t(x) = g(x) */
        COPY_POLY(gx, tx);
    }
    COPY_POLY(sx, gx);
    
    free_poly(sx);
    free_poly(tx);

    return 1;
}

z_poly* extended_euclid_poly(const z_poly* ax, const z_poly *bx,
                             z_poly **sx, z_poly **tx)
{
    /**
       def xgcd(a,b):
           x_0,x_1=1,0;
           y_0,y_1=0,1
           while b:
               q = a/b
               x_1, x_0 = x_0 - q*x_1, x_1
               y_1, y_0 = y_0 - q*y_1, y_1
               a, b = b, a % b
           return a, x_0, y_0
     **/
    
    int i, size;
    z_poly *fx, *gx, *qx, *rx;
    z_poly *x0, *x1, *y0, *y1;
    z_poly *cx, *dx;
    
    size = ax->size;
    if (size < bx->size)
        size = bx->size;
    x0 = init_poly(size, ax->mod); x0->degree = 0; x0->coeff[0] = 1;
    x1 = init_poly(size, ax->mod);
    y0 = init_poly(size, ax->mod);
    y1 = init_poly(size, ax->mod); y1->degree = 0; y1->coeff[0] = 1;
    
    /**
     * Note that we MUST NOT use clone_poly() method here
     * as we want to ensure that both fx and gx has coefficient
     * buffer of size 'size', i.e. MAX(ax->size, bx->size)
     */
    fx = init_poly(size, ax->mod);
    COPY_POLY(ax, fx);
    gx = init_poly(size, ax->mod);
    COPY_POLY(bx, gx);
    while (!is_zero_poly(gx)) {
        /* qx, rx = fx / gx */
        divide_poly(fx, gx, &qx, &rx);
        
        /* cx = x0 - (qx * x1) */
        cx = clone_poly(x0);
        dx = multiply_poly(qx, x1);
        for (i=0; i<=dx->degree; i++) {
            if (dx->coeff[i]) {
                cx->coeff[i] -= dx->coeff[i];
                cx->coeff[i] = int_mod(cx->coeff[i], cx->mod);
            }
        }
        if (cx->degree < dx->degree)
            cx->degree = dx->degree;
        while ((cx->degree >= 0) && !cx->coeff[cx->degree])
            --cx->degree;
        free_poly(dx);
        
        /* x0 = x1 */
        COPY_POLY(x1, x0);
        
        /* x1 = cx */
        COPY_POLY(cx, x1);
        free_poly(cx);
        
        /* cx = y0 - (qx * y1) */
        cx = clone_poly(y0);
        dx = multiply_poly(qx, y1);
        for (i=0; i<=dx->degree; i++) {
            if (dx->coeff[i]) {
                cx->coeff[i] -= dx->coeff[i];
                cx->coeff[i] = int_mod(cx->coeff[i], cx->mod);
            }
        }
        if (cx->degree < dx->degree)
            cx->degree = dx->degree;
        while ((cx->degree >= 0) && !cx->coeff[cx->degree])
            --cx->degree;
        free_poly(dx);
        
        /* y0 = y1 */
        COPY_POLY(y1, y0);
        
        /* y1 = cx */
        COPY_POLY(cx, y1);
        free_poly(cx);
        
        /* fx = gx */
        COPY_POLY(gx, fx);
        COPY_POLY(rx, gx);
        
        free_poly(qx);
        free_poly(rx);
    }
    
    free_poly(gx);
    free_poly(x1);
    free_poly(y1);
    
    if (sx)
        *sx = x0;
    else
        free_poly(x0);
    if (tx)
        *tx = y0;
    else
        free_poly(y0);
    
    return fx;
}

z_poly* inverse_polynomial(const z_poly *ax, const z_poly *mx)
{
    int i;
    coeff_t b;
    z_poly *gx, *ix;
    z_poly *inverse_poly = NULL;
    
    gx = extended_euclid_poly(ax, mx, &ix, NULL);
    
    if (gx->degree == 0 && gx->coeff[0])
        inverse_poly = ix;
    
    if (inverse_poly && gx->coeff[0] > 1) {
        /**
         * Adjust the inverse polynomial so that
         * the product of ax and its inverse is 1
         **/
        b = (coeff_t)modulo_inverse_prime(gx->coeff[0], gx->mod);
        for (i=0; i<=inverse_poly->degree; i++)
            inverse_poly->coeff[i] = int_mod(inverse_poly->coeff[i] * b, gx->mod);
    }
    
    free_poly(gx);
    if (inverse_poly == NULL)
        free_poly(ix);
    
    return inverse_poly;
}

int is_poly_equal(const z_poly *ax, const z_poly *bx)
{
    return (ax->degree == bx->degree) && (ax->mod == bx->mod) &&
        (!memcmp(ax->coeff, bx->coeff, (ax->degree+1)*sizeof(coeff_t)));
}

char* string_from_poly(const z_poly *ax)
{
    coeff_t a;
    char *str = NULL;
    size_t size = 0;
    int i, found = 0;
    
    if (!ax)
        return NULL;
    
    size = 20*ax->size + 1;
    str = (char *)calloc(size, sizeof(char));
    if (!str)
        return NULL;
    
    if (is_zero_poly(ax))
        return str;
    
    for (i=0; !found; i++) {
        a = ax->coeff[i];
        if (!a)
            continue;
        switch (i) {
            case 0: snprintf(str, size, "%d", a); break;
            case 1: snprintf(str, size, "%dx", a); break;
            default: snprintf(str, size, "%dx^%d", a, i); break;
        }
        found = 1;
    }
    for (; i<=ax->degree; i++) {
        a = ax->coeff[i];
        if (!a)
            continue;
        snprintf(str, size, "%s %s ", str, (a < 0) ? "-" : "+");
        if (a == 1 || a == -1) {
            snprintf(str, size, "%sx", str);
        } else {
            snprintf(str, size, "%s%dx", str, (a < 0) ? -a : a);
        }
        if (i > 1) {
            snprintf(str, size, "%s^%d", str, i);
        }
    }

    return str;
}

void print_poly_as_list(FILE *fptr, const char *prefix, const z_poly *ax, const char *suffix)
{
    int i;
    
    if (!ax)
        return;
    
    if (prefix)
        fprintf(fptr, "%s", prefix);
    fprintf(fptr, "(%d, %d) [%d", ax->degree, ax->size, ax->coeff[0]);
    for (i=1; i<ax->size; i++)
        fprintf(fptr, ", %d", ax->coeff[i]);
    fprintf(fptr, "]\n");

    if (suffix)
        fprintf(fptr, "%s", suffix);

    fflush(fptr);
}
