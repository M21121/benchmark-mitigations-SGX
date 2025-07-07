// mitigations.h
#ifndef MITIGATIONS_H
#define MITIGATIONS_H

#include "mitigation_config.h"
#include <stddef.h>
#include <stdint.h>

void set_enclave_config(const MitigationConfig* config);

namespace mitigations {
    void lfence_barrier();
    void mfence_barrier();
    void cache_flush(const void* addr, size_t size);
    void memory_barrier();
    void constant_time_memcpy(void* dest, const void* src, size_t n);
    void secure_memzero(void* ptr, size_t len);
}

#endif // MITIGATIONS_H