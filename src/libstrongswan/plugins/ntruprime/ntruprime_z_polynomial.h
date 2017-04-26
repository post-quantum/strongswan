/**
 *  z_polynomial.h
 *  ntrup
 *
 *  Created by CJ on 11/01/2017.
 *  Copyright © 2017 Post-Quantum. All rights reserved.
 **/

#ifndef _POLYNOMIAL_H
#define _POLYNOMIAL_H

#include <stdint.h>
typedef int32_t coeff_t;

typedef struct {
    int size, degree;
    coeff_t *coeff, mod;
} z_poly;

/**
 *  Initialises an integer polynomial of at most `size` coefficients
 *  and each coefficient takes value from 0 to `mod`-1.
 *
 *  @param[in] size The maximum number of coefficients
 *  @param[in] mod  The modulus of the coefficients
 *  @return an integer polynomial on success, NULL on failure
 **/
z_poly* init_poly(int size, coeff_t mod);

/**
 *  Deinitiaises an integer polynomial
 *
 *  @param[in] px The integer polynomial
 **/
void free_poly(z_poly* px);

/**
 *  Updates the degree of the integer polynomial
 *
 *  @param[in] ax The integer polynomial
 **/
void update_poly_degree(z_poly *ax);

/**
 *  Zeroes an integer polynomial
 *
 *  @param[in] px The integer polynomial
 **/
void zero_poly(z_poly* px);

/**
 *  Returns whether or not the integer polynomial is zero
 *
 *  @param[in] px The integer polynomial
 *  @return 1 if the polynomial is zero, 0 otherwise
 **/
int is_zero_poly(const z_poly *px);

/**
 *  Clones an integer polynomial
 *
 *  @param[in] px The integer polynomial
 *  @return a cloned integer polynomial
 **/
z_poly* clone_poly(const z_poly *px);

/**
 *  Normalises the coefficients of polynomial p(x)
 *  so that they are in the range {0,1,...,q-1}
 *  where q is the modulo size of the coefficient
 *
 *  @param[in,out] px The input/output polynomial
 **/
void normalise_poly(z_poly *px);

/**
 *  Create a random integer polynomial with constraints on the
 *  posible coefficient values, weight, and degree
 *
 *  @param[in] size   The number of elements in the polynomial
 *  @param[in] mod    The modulo of the polynomial coefficients
 *  @param[in] c      The possible coefficient values
 *  @param[in] c_size The number of coefficients
 *  @param[in] weight The weight of the polynomial, 0 for random
 *  @param[in] degree The degree of the polynomial, 0 for unspecified
 *                    otherwise the returned polynomial will be
 *                    guaranteed to have the specified degree
 *  @return an integer polynomial meeting the constraints
 **/
z_poly *create_random_poly_with_constraints(int size,
                                            coeff_t mod,
                                            const coeff_t *c,
                                            size_t c_size,
                                            int weight,
                                            int degree);

/**
 *  Performs modulo p reduction on each coefficient
 *  of polynomial a(x).
 *
 *  @param[in,out] ax The input/output polynomial
 *  @param[in]     p  The prime p for modulo reduction
 **/
void coeff_mod_poly(z_poly *ax, coeff_t p);

/**
 *  Multiplies two integer polynomials
 *
 *  @param[in] ax The multiplicand integer polynomial
 *  @param[in] bx The multiplier integer polynomial
 *  @return an integer polynomial that is the product of both inputs
 **/
z_poly *multiply_poly(const z_poly *ax, const z_poly *bx);

/**
 *  Divides two integer polynomials to obtain 
 *  a quotient and reminder polynomials
 *
 *  @param[in]  ax The dividend integer polynomial
 *  @param[in]  bx The divisor integer polynomial
 *  @param[out] qx The pointer to quotient integer polynomial
 *  @param[out] rx The pointer to reminder integer polynomial
 *  @return 1 on success, 0 otherwise
 **/
int divide_poly(const z_poly *ax, const z_poly *bx, z_poly **qx, z_poly **rx);

/**
 *  Modulo reduction of an integer polynomial
 *
 *  The method performs the following operation:
 *      a(x) = a(x) mod m(x)
 *
 *  @param[in]     mx The modulo polynomial
 *  @param[in,out] ax The input/output polynomial
 **/
void modulo_reduce_poly(const z_poly *mx, z_poly *ax);

/**
 *  Obtain the GCD of two integer polynomials
 *
 *  The method performs the following operation:
 *      g(x) = GCD(a(x), b(x))
 *
 *  @param[in]  ax The left input polynomial
 *  @param[in]  bx The right input polynomial
 *  @param[out] gx The output polynomial
 *  @return 1 on successful operation, 0 otherwise
 **/
int gcd_poly(const z_poly* ax, const z_poly *bx, z_poly *gx);

/**
 *  Runs extended Euclid algorithm between two integer polynomials
 *  `ax` and `bx`, to obtain integer polynomials `sx`, `tx`, and
 *  `gx`, such that the following condition is met:
 *      `ax`.`sx` + `bx`.`tx` = `gx`
 *
 *  @note If both pointers `sx` and tx` are set to NULL, this
 *  method is equal to `gcd_poly` method.
 *
 *  @param[in]  ax The input integer polynomial
 *  @param[in]  bx The input integer polynomial
 *  @param[out] sx The nullable pointer to an output integer polynomial
 *  @param[out] tx The nullable pointer to an output integer polynomial
 *  @return an integer polynomial `gx`
 **/
z_poly* extended_euclid_poly(const z_poly* ax, const z_poly *bx,
                             z_poly **sx, z_poly **tx);

/**
 *  Calculates the inverse of an integer polynomial modulo
 *  another integer polynomial.
 *
 *  @param[in] ax The input integer polynomial
 *  @param[in] mx The input modulus integer polynomial
 *  @return an inverse integer polynomial
 **/
z_poly* inverse_polynomial(const z_poly *ax, const z_poly *mx);

/**
 *  Returns whether or not two integer polynomials are equal
 *
 *  @param[in] ax The input integer polynomial
 *  @param[in] bx Another input integer polynomial
 *  @return 1 if both polynomials are equal, 0 otherwise
 **/
int is_poly_equal(const z_poly *ax, const z_poly *bx);

/**
 *  Returns a string representation of an integer polynomial
 *
 *  @param[in] ax The input integer polynomial
 *  @return a string representation of the polynomial
 **/
char* string_from_poly(const z_poly *ax);

void print_poly_as_list(FILE *fptr, const char *prefix, const z_poly *ax, const char *suffix);

#endif /* _POLYNOMIAL_H */
