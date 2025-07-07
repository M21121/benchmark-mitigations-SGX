// app/benchmark_runner.h
#ifndef BENCHMARK_RUNNER_H
#define BENCHMARK_RUNNER_H

#include <string>
#include <vector>
#include <cstdint>

struct BenchmarkResult {
    double time_ms;
    uint64_t cycles;
    double cycles_per_op;
};

class BenchmarkRunner {
private:
    void flush_caches();

public:
    void setup_environment();
    BenchmarkResult benchmark_empty_ecall(int iterations);
    BenchmarkResult benchmark_pure_ocall(int iterations);
    BenchmarkResult benchmark_ping_pong(int iterations);
    BenchmarkResult benchmark_file_read(const std::string& filename, int iterations);
    BenchmarkResult benchmark_sgx_file_read(const std::string& filename, int iterations);
    BenchmarkResult benchmark_crypto_workload(int iterations);
    void create_sealed_test_file(const std::string& filename);
};

#endif // BENCHMARK_RUNNER_H
