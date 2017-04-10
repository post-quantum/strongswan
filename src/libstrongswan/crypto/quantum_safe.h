/*
 * Experimental code from Post-Quantum Ltd
 */

/**
 * @defgroup quantum_safe quantum_safe
 * @{ @ingroup crypto
 */

#ifndef QUANTUM_SAFE_GROUP_H_
#define QUANTUM_SAFE_GROUP_H_

#ifdef QSKE

typedef enum quantum_safe_group_t quantum_safe_group_t;
typedef struct quantum_safe_t quantum_safe_t;

#include <library.h>

/**
 * Quantum-safe groups
 */
enum quantum_safe_group_t {
	QS_NONE     =  0,
	QS_NTRU_112_BIT = 1,
	QS_NTRU_128_BIT = 2,
	QS_NTRU_192_BIT = 3,
	QS_NTRU_256_BIT = 4,
	QS_NH_128_BIT   = 16,
	QS_NTRU_PRIME_129_BIT = 32,
};

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
