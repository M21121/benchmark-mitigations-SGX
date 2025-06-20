// enclave.cpp

#include "enclave_t.h"
#include "mitigations.h"
#include "mitigation_config.h"
#include "sgx_tseal.h"
#include <string.h>

extern MitigationConfig g_enclave_config;

void perform_stable_workload() {
    volatile int counter = 0;
    for (int i = 0; i < 100; ++i) {
        counter += i % 123;
    }
}

void ecall_warmup() {
    perform_stable_workload();
}

void ecall_set_mitigation_config(const MitigationConfig* config) {
    set_enclave_config(config);
}

void apply_speculation_mitigations() {
    mitigations::lfence_barrier();
    mitigations::mfence_barrier();
}

void ecall_empty() {
    if (g_enclave_config.lfence_barrier || g_enclave_config.mfence_barrier) {
        apply_speculation_mitigations();
    }

    if (g_enclave_config.timing_noise) {
        mitigations::timing_noise();
    }

    perform_stable_workload();

    if (g_enclave_config.memory_barriers) {
        mitigations::memory_barrier();
    }
}


void ecall_ping(int iteration) {
    apply_speculation_mitigations();
    mitigations::timing_noise();
    pong_ocall(iteration);
    mitigations::timing_noise();
}

void ecall_trigger_ocall() {
    apply_speculation_mitigations();
    mitigations::timing_noise();
    empty_ocall();
    apply_speculation_mitigations();
    mitigations::timing_noise();
}

// Add to enclave.cpp
void ecall_setup_ocall_benchmark() {
    // Setup any necessary state for OCALL measurement
    apply_speculation_mitigations();
}

void ecall_measure_pure_ocall(int iterations) {
    // This function runs inside the enclave and measures pure OCALL overhead
    apply_speculation_mitigations();
    mitigations::timing_noise();

    // The timing is done on the app side, so we just make the OCALLs here
    for (int i = 0; i < iterations; i++) {
        empty_ocall();
        // Apply mitigations between calls if configured
        if (i % 100 == 0) {
            apply_speculation_mitigations();
        }
    }

    mitigations::timing_noise();
}

// METHOD 1: Untrusted File I/O (fast, applies mitigations conditionally)
void ecall_file_read(const char* filename) {
    apply_speculation_mitigations();
    mitigations::timing_noise();

    char buffer[8192] = {0};
    size_t bytes_read = 0;

    mitigations::cache_flush(buffer, sizeof(buffer));
    ocall_read_file(&bytes_read, filename, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        volatile uint32_t checksum = 0;
        for (size_t i = 0; i < bytes_read; i++) {
            checksum += (unsigned char)buffer[i];
            if (i % 64 == 0) {
                apply_speculation_mitigations();
            }
        }

        if (g_enclave_config.cache_flushing) {
            mitigations::cache_flush(buffer, bytes_read);
        }
        if (g_enclave_config.constant_time_ops) {
            mitigations::secure_memzero(buffer, sizeof(buffer));
        }
    }

    mitigations::timing_noise();
}

// METHOD 2: SGX Sealed Data (slow, hardware encryption, applies mitigations conditionally)
void ecall_sgx_file_read(const char* filename) {
    apply_speculation_mitigations();
    mitigations::timing_noise();

    const size_t plain_size = 4096;
    const size_t sealed_overhead = sgx_calc_sealed_data_size(0, plain_size);
    uint8_t sealed_buffer[sealed_overhead];
    size_t sealed_bytes_read = 0;

    mitigations::cache_flush(sealed_buffer, sizeof(sealed_buffer));
    ocall_read_sealed_file(&sealed_bytes_read, filename, sealed_buffer, sizeof(sealed_buffer));

    if (sealed_bytes_read > 0 && sealed_bytes_read <= sizeof(sealed_buffer)) {
        char unsealed_buffer[plain_size] = {0};
        uint32_t unsealed_len = sizeof(unsealed_buffer);

        // SGX unsealing (always - this is what makes it "sgx_file_read")
        sgx_status_t ret = sgx_unseal_data(
            (const sgx_sealed_data_t*)sealed_buffer,
            NULL, NULL,
            (uint8_t*)unsealed_buffer, &unsealed_len
        );

        if (ret == SGX_SUCCESS && unsealed_len > 0) {
            volatile uint32_t checksum = 0;
            for (size_t i = 0; i < unsealed_len; i++) {
                checksum += (unsigned char)unsealed_buffer[i];
                if (i % 64 == 0) {
                    apply_speculation_mitigations();
                }
            }

            // Always secure cleanup for sealed data
            mitigations::secure_memzero(unsealed_buffer, sizeof(unsealed_buffer));
        }
    }

    // Always secure cleanup for sealed operations
    mitigations::secure_memzero(sealed_buffer, sizeof(sealed_buffer));
    mitigations::timing_noise();
}

// Helper function to create sealed test files
void ecall_create_sealed_file(const char* filename, const char* data, size_t data_len) {
    apply_speculation_mitigations();

    const size_t max_data_len = 4096;
    uint32_t actual_len = (data_len > max_data_len) ? max_data_len : static_cast<uint32_t>(data_len);

    uint32_t sealed_size = sgx_calc_sealed_data_size(0, actual_len);
    uint8_t* sealed_buffer = new uint8_t[sealed_size];

    if (sealed_buffer) {
        sgx_status_t ret = sgx_seal_data(
            0, NULL,
            actual_len, (const uint8_t*)data,
            sealed_size, (sgx_sealed_data_t*)sealed_buffer
        );

        if (ret == SGX_SUCCESS) {
            int write_result = 0;
            ocall_write_sealed_file(&write_result, filename, sealed_buffer, sealed_size);
        }

        delete[] sealed_buffer;
    }
}

void ecall_crypto_workload() {
    apply_speculation_mitigations();
    mitigations::timing_noise();

    const size_t data_size = 4096;
    char buffer[data_size];
    char hash_output[32];

    for (size_t i = 0; i < data_size; i++) {
        buffer[i] = (char)(i * 17 + 42);
    }

    uint32_t hash = 0x12345678;
    for (size_t i = 0; i < data_size; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)buffer[i];
        if (i % 128 == 0) {
            apply_speculation_mitigations();
        }
    }

    for (int round = 0; round < 100; round++) {
        for (size_t i = 0; i < 32; i++) {
            hash_output[i] = (char)((hash >> (i % 32)) ^ (round * i));
        }
        hash = ((hash << 3) + hash) ^ round;
    }

    mitigations::cache_flush(buffer, data_size);
    mitigations::cache_flush(hash_output, 32);
    mitigations::secure_memzero(buffer, data_size);

    mitigations::timing_noise();
}
