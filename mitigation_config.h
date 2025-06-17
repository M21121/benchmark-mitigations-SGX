// mitigation_config.h
#ifndef MITIGATION_CONFIG_H
#define MITIGATION_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // Individual speculation barriers
    bool lfence_barrier;
    bool mfence_barrier;
    bool cpuid_barrier;

    // Other existing mitigations
    bool cache_flushing;
    bool timing_noise;
    bool constant_time_ops;
    bool memory_barriers;
    bool disable_hyperthreading;

    // Legacy flag for backward compatibility
    bool speculation_barriers;
} MitigationConfig;

static inline void init_mitigation_config(MitigationConfig* config) {
    if (config) {
        config->lfence_barrier = false;
        config->mfence_barrier = false;
        config->cpuid_barrier = false;
        config->cache_flushing = false;
        config->timing_noise = false;
        config->constant_time_ops = false;
        config->memory_barriers = false;
        config->disable_hyperthreading = false;
        config->speculation_barriers = false;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void set_enclave_config(const MitigationConfig* config);

#ifdef __cplusplus
}
#endif

#endif // MITIGATION_CONFIG_H
