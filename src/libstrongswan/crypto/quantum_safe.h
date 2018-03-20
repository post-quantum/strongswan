/*
 * Experimental code from Post-Quantum Ltd
 */

/**
 * @defgroup quantum_safe quantum_safe
 * @{ @ingroup crypto
 */

#ifndef QUANTUM_SAFE_GROUP_H_
#define QUANTUM_SAFE_GROUP_H_


typedef enum quantum_safe_group_t quantum_safe_group_t;
typedef struct quantum_safe_t quantum_safe_t;

#include <library.h>

/**
 * Quantum-safe groups
 */
enum quantum_safe_group_t {
	QS_NONE     =  0,
	QS_NH_128_BIT   = 16,
	QS_NTRU_112_BIT = 32,
	QS_NTRU_128_BIT = 33,
	QS_NTRU_192_BIT = 34,
	QS_NTRU_256_BIT = 35,
	QS_NTRU_PRIME_129_BIT = 64,
    // NB: the below must match the DH versions
    QS_NIST_PQC_NEWHOPE512CCA = 2048,
    QS_NIST_PQC_KYBER512 = 2049,
    QS_NIST_PQC_NTRULPR4591761 = 2050,
    QS_NIST_PQC_NTRUKEM443 = 2051,
    QS_NIST_PQC_SIKEP503 = 2052,
    QS_NIST_PQC_LEDAKEM128SLN02 = 2053,
};


#ifdef QSKE

/**
 * enum name for quantum_safe_group_t.
 */
extern enum_name_t *quantum_safe_group_names;

/**
 * Interface for quantum safe key exchange, as in RFC#tba.
 */
struct quantum_safe_t {

	quantum_safe_group_t (*get_qs_group) (quantum_safe_t *this);

	bool (*get_shared_secret)(quantum_safe_t *this, chunk_t *secret)
		__attribute__((warn_unused_result));

	bool (*set_other_public_value)(quantum_safe_t *this, chunk_t value)
		__attribute__((warn_unused_result));

	bool (*get_my_public_value) (quantum_safe_t *this, chunk_t *value)
		__attribute__((warn_unused_result));

	bool (*set_private_value)(quantum_safe_t *this, chunk_t value)
		__attribute__((warn_unused_result));

	void (*destroy) (quantum_safe_t *this);
};

#endif

#endif /** QUANTUM_SAFE_GROUP_H_ @}*/
