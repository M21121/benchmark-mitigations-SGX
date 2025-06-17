// mitigations.h
#ifndef MITIGATIONS_H
#define MITIGATIONS_H

#include "mitigation_config.h"
#include <stddef.h>

namespace mitigations {
    void lfence_barrier();
    void mfence_barrier();
    void cpuid_barrier();
    void speculation_barrier();
    void cache_flush(const void* addr, size_t size);
    void timing_noise();
    void memory_barrier();
    void constant_time_memcpy(void* dest, const void* src, size_t n);
    void secure_memzero(void* ptr, size_t len);
    uint32_t get_random_delay();
}

#endif // MITIGATIONS_H
