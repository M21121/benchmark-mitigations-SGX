// app.cpp
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sched.h>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include "sgx_urts.h"
#include "enclave_u.h"
#include "mitigation_config.h"

// Declare the app-side config, defined in app_config.cpp
extern MitigationConfig g_app_config;

sgx_enclave_id_t global_eid = 0;

// Forward declaration
void set_mitigation_flag(const std::string& flag);

class BenchmarkRunner {
private:
    std::vector<int> physical_cores;

    // Helper to identify physical cores (assumes hyperthreading is paired on adjacent cores)
    void get_physical_cores() {
        long num_cores_long = sysconf(_SC_NPROCESSORS_ONLN);
        int num_cores = static_cast<int>(num_cores_long);
        // This simple logic assumes core 0, 2, 4... are physical cores
        for (int i = 0; i < num_cores; i += 2) {
            physical_cores.push_back(i);
        }
    }

    // Pin the current thread to a specific physical core to mitigate hyperthreading effects
    void pin_to_physical_core() {
        if (physical_cores.empty()) get_physical_cores();

        if (physical_cores.empty()) {
            std::cerr << "Warning: Could not determine physical cores." << std::endl;
            return;
        }

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(physical_cores[0], &cpuset);

        if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0) {
            std::cout << "Pinned to physical core " << physical_cores[0] << std::endl;
        } else {
            std::cerr << "Warning: Failed to pin to physical core." << std::endl;
        }
    }

public:
    // Set up environment based on configuration, like pinning cores
    void setup_environment() {
        if (g_app_config.disable_hyperthreading) {
            pin_to_physical_core();
        }
        // Send the final configuration into the enclave
        ecall_set_mitigation_config(global_eid, &g_app_config);
    }

    // Benchmark a minimal ECALL to measure transition overhead + enabled mitigations
    double benchmark_empty_ecall(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_empty(global_eid);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    // Correctly benchmark an OCALL by triggering it from within the enclave
    double benchmark_pure_ocall(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_trigger_ocall(global_eid);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    // Benchmark an ECALL that immediately makes an OCALL back to the app
    double benchmark_ping_pong(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_ping(global_eid, i);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    // Benchmark file reading via an OCALL
    double benchmark_file_read(const std::string& filename, int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_file_read(global_eid, filename.c_str());
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    // Benchmark file reading with SGX-specific data processing mitigations
    double benchmark_sgx_file_read(const std::string& filename, int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_sgx_file_read(global_eid, filename.c_str());
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    double benchmark_crypto_workload(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            ecall_crypto_workload(global_eid);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count()) / 1000.0;
    }
};

// Helper function to set a specific mitigation flag
void set_mitigation_flag(const std::string& flag) {
    if (flag == "lfence") g_app_config.lfence_barrier = true;
    else if (flag == "mfence") g_app_config.mfence_barrier = true;
    else if (flag == "cpuid") g_app_config.cpuid_barrier = true;
    else if (flag == "speculation") g_app_config.speculation_barriers = true;
    else if (flag == "cache") g_app_config.cache_flushing = true;
    else if (flag == "timing") g_app_config.timing_noise = true;
    else if (flag == "constant") g_app_config.constant_time_ops = true;
    else if (flag == "memory") g_app_config.memory_barriers = true;
    else if (flag == "hyperthreading") g_app_config.disable_hyperthreading = true;
}

void parse_mitigations(const std::string& mitigation_str) {
    init_mitigation_config(&g_app_config);

    if (mitigation_str.empty() || mitigation_str == "none") return;
    if (mitigation_str == "all") {
        g_app_config.speculation_barriers = true;
        g_app_config.cache_flushing = true;
        g_app_config.timing_noise = true;
        g_app_config.constant_time_ops = true;
        g_app_config.memory_barriers = true;
        g_app_config.disable_hyperthreading = true;
        return;
    }
    if (mitigation_str == "all_speculation") {
        g_app_config.lfence_barrier = true;
        g_app_config.mfence_barrier = true;
        g_app_config.cpuid_barrier = true;
        return;
    }

    std::string current_token;
    std::string remaining = mitigation_str + ",";
    size_t pos = 0;
    while ((pos = remaining.find(',')) != std::string::npos) {
        current_token = remaining.substr(0, pos);
        if (!current_token.empty()) {
            set_mitigation_flag(current_token);
        }
        remaining.erase(0, pos + 1);
    }
}

void print_config() {
    std::cout << "Current mitigation configuration:\n";
    std::cout << "  LFENCE barrier:       " << (g_app_config.lfence_barrier ? "ON" : "OFF") << "\n";
    std::cout << "  MFENCE barrier:       " << (g_app_config.mfence_barrier ? "ON" : "OFF") << "\n";
    std::cout << "  CPUID barrier:        " << (g_app_config.cpuid_barrier ? "ON" : "OFF") << "\n";
    std::cout << "  Speculation barriers: " << (g_app_config.speculation_barriers ? "ON" : "OFF") << "\n";
    std::cout << "  Cache flushing:       " << (g_app_config.cache_flushing ? "ON" : "OFF") << "\n";
    std::cout << "  Timing noise:         " << (g_app_config.timing_noise ? "ON" : "OFF") << "\n";
    std::cout << "  Constant time ops:    " << (g_app_config.constant_time_ops ? "ON" : "OFF") << "\n";
    std::cout << "  Memory barriers:      " << (g_app_config.memory_barriers ? "ON" : "OFF") << "\n";
    std::cout << "  Disable hyperthreading: " << (g_app_config.disable_hyperthreading ? "ON" : "OFF") << "\n";
}

// --- OCALL Implementations ---
// These functions are called by the enclave to interact with the untrusted app

// OCALL to perform some work, ensuring the OCALL transition itself is measured
void empty_ocall() {
    // Perform a small amount of volatile work to prevent this from being optimized away
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i;
    }
}

// OCALL to print a string from the enclave
void ocall_print_string(const char* str) {
    // Using printf for direct output. In a real app, use a safer logging mechanism.
    printf("%s", str);
}

// OCALL part of the ping-pong test
void pong_ocall(int iteration) {
    // This OCALL intentionally does nothing but return, to measure the transition
}

// OCALL to read a file from the untrusted filesystem
size_t ocall_read_file(const char* filename, char* buf, size_t buf_len) {
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;

    size_t bytes_read = fread(buf, 1, buf_len, file);
    fclose(file);
    return bytes_read;
}

// Initializes the SGX enclave
int initialize_enclave() {
    sgx_status_t ret;
    sgx_launch_token_t token = {0};
    int updated = 0;

    ret = sgx_create_enclave("enclave.signed.so", SGX_DEBUG_FLAG, &token, &updated, &global_eid, nullptr);
    return (ret == SGX_SUCCESS) ? 0 : -1;
}

void print_usage(const char* program) {
    std::cout << "Usage: " << program << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -t, --test TYPE          Test type (ecall, pure_ocall, pingpong, fileread, sgxread)\n";
    std::cout << "  -i, --iterations N       Number of iterations (default: 1000)\n";
    std::cout << "  -f, --file FILE          File for read tests (default: test.txt)\n";
    std::cout << "  -m, --mitigations LIST   Comma-separated mitigations (e.g., speculation,cache,all,none)\n";
    std::cout << "  -o, --output FILE        Output CSV file\n";
    std::cout << "  -h, --help               Show this help\n";
}

int main(int argc, char* argv[]) {
    std::string test_type;
    int iterations = 1000;
    std::string filename = "test.txt";
    std::string output_file;
    std::string mitigations = "none";

    static struct option long_options[] = {
        {"test", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"file", required_argument, 0, 'f'},
        {"mitigations", required_argument, 0, 'm'},
        {"output", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "t:i:f:m:o:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 't': test_type = optarg; break;
            case 'i': iterations = std::stoi(optarg); break;
            case 'f': filename = optarg; break;
            case 'm': mitigations = optarg; break;
            case 'o': output_file = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: print_usage(argv[0]); return 1;
        }
    }

    if (test_type.empty()) {
        std::cerr << "Error: Test type required\n";
        print_usage(argv[0]);
        return 1;
    }

    parse_mitigations(mitigations);
    print_config();

    if (initialize_enclave() < 0) {
        std::cerr << "Failed to initialize enclave\n";
        return 1;
    }

    BenchmarkRunner runner;
    runner.setup_environment();

    // --- WARM-UP PHASE ---
    // Run the warm-up ECALL a number of times to stabilize the CPU frequency.
    std::cout << "Warming up CPU..." << std::endl;
    for (int i = 0; i < 200; ++i) { // Run many iterations
         ecall_warmup(global_eid);
    }
    std::cout << "Warm-up complete. Starting benchmark." << std::endl;
    // --- END WARM-UP PHASE ---

    double time_ms = 0;

    if (test_type == "ecall") {
        time_ms = runner.benchmark_empty_ecall(iterations);
    } else if (test_type == "pure_ocall") {
        time_ms = runner.benchmark_pure_ocall(iterations);
    } else if (test_type == "pingpong") {
        time_ms = runner.benchmark_ping_pong(iterations);
    } else if (test_type == "fileread") {
        time_ms = runner.benchmark_file_read(filename, iterations);
    } else if (test_type == "sgxread") {
        time_ms = runner.benchmark_sgx_file_read(filename, iterations);
    } else if (test_type == "crypto") {
        time_ms = runner.benchmark_crypto_workload(iterations);
    } else {
        std::cerr << "Unknown test type: " << test_type << "\n";
        sgx_destroy_enclave(global_eid);
        return 1;
    }

    double time_per_op = (time_ms * 1000.0) / iterations;
    std::cout << "Results: " << time_ms << "ms total, " << time_per_op << "μs per operation\n";

    if (!output_file.empty()) {
        std::ofstream csv(output_file, std::ios::app);
        csv << test_type << "," << mitigations << "," << iterations << ","
            << time_ms << "," << time_per_op << "\n";
    }

    sgx_destroy_enclave(global_eid);
    return 0;
}