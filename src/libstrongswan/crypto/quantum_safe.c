/*
 * Experimental code from Post-Quantum
 */

#include "quantum_safe.h"

#ifdef QSKE

ENUM_BEGIN(quantum_safe_group_names, QS_NONE, QS_NTRU_256_BIT,
	"QS_NONE",
	"QS_NTRU_112_BIT",
	"QS_NTRU_128_BIT",
	"QS_NTRU_192_BIT",
	"QS_NTRU_256_BIT");
ENUM_NEXT(quantum_safe_group_names, QS_NH_128_BIT, QS_NH_128_BIT, QS_NTRU_256_BIT,
	"QS_NH_128_BIT");
ENUM_NEXT(quantum_safe_group_names, QS_NTRU_PRIME_129_BIT, QS_NTRU_PRIME_129_BIT, QS_NH_128_BIT,
	"QS_NTRU_PRIME_129_BIT");
ENUM_END(quantum_safe_group_names, QS_NTRU_PRIME_129_BIT);


#endif
