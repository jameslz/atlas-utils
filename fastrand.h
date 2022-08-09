#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/*
pgc
*/

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
    // selected. Must *always* be odd.
};

typedef struct pcg_state_setseq_64 pcg32_random_t;

static inline uint32_t pcg32_random_r(pcg32_random_t* rng);

static inline uint32_t pcg32_random(void);

void pcg32_init_state(uint32_t state);

static inline void pcg32_init_inc(uint32_t inc);

uint32_t pcg32_random_bounded_divisionless(uint32_t range);