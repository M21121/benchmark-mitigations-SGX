// app/benchmark_runner.cpp
#include "benchmark_runner.h"
#include "cycle_counter.h"
#include "enclave_u.h"
#include "mitigation_config.h"
#include <chrono>
#include <iostream>

extern sgx_enclave_id_t global_eid;
extern MitigationConfig g_app_config;

void BenchmarkRunner::flush_caches() {
    const size_t cache_flush_size = 32 * 1024 * 1024;
    volatile char* flush_buffer = new char[cache_flush_size];

    for (size_t i = 0; i < cache_flush_size; i += 64) {
        flush_buffer[i] = static_cast<char>(i & 0xFF);
    }

    volatile int sum = 0;
    for (size_t i = 0; i < cache_flush_size; i += 64) {
        sum += flush_buffer[i];
    }

    delete[] flush_buffer;
    __asm__ volatile ("mfence" ::: "memory");
}

void BenchmarkRunner::setup_environment() {
    ecall_set_mitigation_config(global_eid, &g_app_config);
}

BenchmarkResult BenchmarkRunner::benchmark_empty_ecall(int iterations) {
    flush_caches();

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        ecall_empty(global_eid);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

BenchmarkResult BenchmarkRunner::benchmark_pure_ocall(int iterations) {
    flush_caches();

    sgx_status_t ret = ecall_setup_ocall_benchmark(global_eid);
    if (ret != SGX_SUCCESS) {
        std::cerr << "Failed to setup OCALL benchmark" << std::endl;
        return {0.0, 0, 0.0};
    }

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    ret = ecall_measure_pure_ocall(global_eid, iterations);

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    if (ret != SGX_SUCCESS) {
        std::cerr << "OCALL benchmark failed" << std::endl;
        return {0.0, 0, 0.0};
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

BenchmarkResult BenchmarkRunner::benchmark_ping_pong(int iterations) {
    flush_caches();

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        ecall_ping(global_eid, i);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

BenchmarkResult BenchmarkRunner::benchmark_file_read(const std::string& filename, int iterations) {
    flush_caches();

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        ecall_file_read(global_eid, filename.c_str());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

BenchmarkResult BenchmarkRunner::benchmark_sgx_file_read(const std::string& filename, int iterations) {
    flush_caches();

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        ecall_sgx_file_read(global_eid, filename.c_str());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

BenchmarkResult BenchmarkRunner::benchmark_crypto_workload(int iterations) {
    flush_caches();

    uint64_t start_cycles = CycleCounter::get_cycles();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        ecall_crypto_workload(global_eid);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = CycleCounter::get_cycles();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    uint64_t total_cycles = end_cycles - start_cycles;

    return {
        static_cast<double>(duration.count()) / 1000.0,
        total_cycles,
        static_cast<double>(total_cycles) / iterations
    };
}

void BenchmarkRunner::create_sealed_test_file(const std::string& filename) {
    std::string test_data = "This is test data for SGX sealing benchmark. ";
    for (int i = 0; i < 50; i++) {
        test_data += "More test data " + std::to_string(i) + ". ";
    }

    std::string sealed_filename = filename + ".sealed";
    ecall_create_sealed_file(global_eid, sealed_filename.c_str(),
                            test_data.c_str(), test_data.length());

    std::cout << "Created sealed test file: " << sealed_filename << std::endl;
}
