#ifndef MITIGATION_CONFIG_H
#define MITIGATION_CONFIG_H

#include <stdbool.h>

typedef struct {
    // Individual speculation barriers
    bool lfence_barrier;
    bool mfence_barrier;

    // Other mitigations
    bool cache_flushing;
    bool timing_noise;
    bool constant_time_ops;
    bool memory_barriers;
    bool disable_hyperthreading;

} MitigationConfig;

static inline void init_mitigation_config(MitigationConfig* config) {
    if (config) {
        config->lfence_barrier = false;
        config->mfence_barrier = false;
        config->cache_flushing = false;
        config->timing_noise = false;
        config->constant_time_ops = false;
        config->memory_barriers = false;
        config->disable_hyperthreading = false;
    }
}

#endif // MITIGATION_CONFIG_H
