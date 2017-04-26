#include <library.h>
#include "ntruprime_pq_random.h"

#define PQ_RAND_RESEED_BUFFER_SIZE      8192
#define PQ_RAND_RESEED_INTERVAL_BYTES   (1UL << 32)

typedef struct {
    rng_t *rng;
    int rdrand_available;
    unsigned long error_code;
    int is_initialised;
    uint64_t counter;
} pq_rand_priv_context;

static pq_rand_priv_context priv_ctx;

int pq_rand_init()
{
    /* Initialise context */
    priv_ctx.rdrand_available = 0;
    priv_ctx.error_code = 0;
    priv_ctx.is_initialised = 0;
    priv_ctx.counter = 0UL;

    priv_ctx.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
    if (!priv_ctx.rng)
    {
        DBG1(DBG_LIB, "could not attach entropy source for RND");
        return PQ_RANDOM_ERROR;
    }

    priv_ctx.rdrand_available = 1;
    priv_ctx.is_initialised = 1;

    return PQ_RANDOM_OK;
}

int pq_rand_deinit()
{
	if (priv_ctx.rng)
		priv_ctx.rng->destroy(priv_ctx.rng);
    priv_ctx.rdrand_available = 0;
    priv_ctx.is_initialised = 0;
    priv_ctx.error_code = 0;
    priv_ctx.counter = 0UL;

    return PQ_RANDOM_OK;
}

int pq_rand_reseed(const uint8_t *seed, size_t seed_size)
{
    /**
     * This function will be silently skipped,
     * it is not supported
     **/
     return PQ_RANDOM_METHOD_NOT_SUPPORTED;
}

int pq_rand_bytes(uint8_t *buf, size_t buf_size)
{
	if (!priv_ctx.rng)
		pq_rand_init();

    if (!priv_ctx.rng->get_bytes(priv_ctx.rng, buf_size, buf))
        return PQ_RANDOM_ERROR;

    return PQ_RANDOM_OK;
}

int pq_rand_uint32_at_most(uint32_t max, uint32_t *rnd)
{
    int status = 0;
    const uint32_t range = 1 + max;
    const uint32_t buckets = RAND_MAX / range;
    const uint32_t limit = buckets * range;

    *rnd = 0;

    /** Note that range must not be larger than or equal to RAND_MAX,
     * otherwise the following loop will be stuck forever
     **/
    if (range >= RAND_MAX)
        return 0;

    /**
     * Use buckets instead of modulo operation to prevent modulo bias
     **/
    do {
        status = pq_rand_bytes((uint8_t *)rnd, sizeof(uint32_t));
    } while (*rnd >= limit && status);
    *rnd /= buckets;

    return status;
}

unsigned long pq_rand_get_last_error_code()
{
    return priv_ctx.error_code;
}
