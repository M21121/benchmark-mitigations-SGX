// enclave.cpp
#include "enclave_t.h"
#include "mitigations.h"

// A simple, stable workload to ensure the CPU is doing real work.
// The `volatile` keyword is crucial to prevent the compiler from optimizing this away.
void perform_stable_workload() {
    volatile int counter = 0;
    for (int i = 0; i < 2000; ++i) {
        counter += i % 123;
    }
}

// A dedicated warm-up function. Call this before the benchmark timer starts.
void ecall_warmup() {
    perform_stable_workload();
}

void ecall_set_mitigation_config(const MitigationConfig* config) {
    set_enclave_config(config);
}

// ecall_empty now includes a stable workload
void ecall_empty() {
    mitigations::speculation_barrier();
    mitigations::timing_noise();

    perform_stable_workload(); // <-- ADDED WORKLOAD

    mitigations::memory_barrier();
}

// The OCALL trigger also performs the workload
void ecall_trigger_ocall() {
    perform_stable_workload(); // <-- ADDED WORKLOAD
    empty_ocall();
}

// The ECALL side of the ping-pong test.
void ecall_ping(int iteration) {
    mitigations::speculation_barrier();
    mitigations::timing_noise();

    pong_ocall(iteration); // Make an OCALL back to the app

    mitigations::timing_noise();
}

// Helper to process a buffer of data with potential mitigations
void process_buffer(char* buffer, size_t bytes_read) {
    uint32_t checksum = 0;
    // This loop simulates processing the data read from the file
    for (size_t i = 0; i < bytes_read; i++) {
        checksum += (unsigned char)buffer[i];
        // Apply speculation barriers periodically if enabled to mitigate side-channel
        // leakage from data access patterns.
        if (i % 64 == 0) {
            mitigations::speculation_barrier();
        }
    }

    // Mitigations applied after processing
    mitigations::cache_flush(buffer, bytes_read);
    mitigations::secure_memzero(buffer, bytes_read); // Uses constant_time_ops and cache_flush internally
}

// ECALL to test file reading. The actual file I/O happens via an OCALL.
void ecall_file_read(const char* filename) {
    mitigations::speculation_barrier();
    mitigations::timing_noise();

    char buffer[8192] = {0};
    size_t bytes_read = 0;

    mitigations::cache_flush(buffer, sizeof(buffer)); // Flush before OCALL

    // OCALL to the app to read the file into the enclave's buffer
    ocall_read_file(&bytes_read, filename, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        process_buffer(buffer, bytes_read);
    }

    mitigations::timing_noise();
}

// This function is identical to ecall_file_read for this benchmark's purpose.
// In a real application, it might use SGX-specific APIs or sealed data.
void ecall_sgx_file_read(const char* filename) {
    mitigations::speculation_barrier();
    mitigations::timing_noise();

    char buffer[8192] = {0};
    size_t bytes_read = 0;

    mitigations::cache_flush(buffer, sizeof(buffer));

    ocall_read_file(&bytes_read, filename, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        process_buffer(buffer, bytes_read);
    }

    mitigations::timing_noise();
}