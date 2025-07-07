// app/app.cpp (simplified main file)
#include <iostream>
#include <string>
#include <getopt.h>
#include <fstream>
#include "sgx_urts.h"
#include "enclave_u.h"
#include "mitigation_config.h"
#include "benchmark_runner.h"
#include "config_parser.h"

extern MitigationConfig g_app_config;
sgx_enclave_id_t global_eid = 0;

static int initialize_enclave() {
    sgx_status_t ret;
    sgx_launch_token_t token = {0};
    int updated = 0;
    ret = sgx_create_enclave("enclave.signed.so", SGX_DEBUG_FLAG,
                             &token, &updated, &global_eid, nullptr);
    return (ret == SGX_SUCCESS) ? 0 : -1;
}

static void print_usage(const char* program) {
    std::cout << "Usage: " << program << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -t, --test TYPE          Test type (ecall, pure_ocall, pingpong, untrusted_file, sealed_file, crypto)\n";
    std::cout << "  -i, --iterations N       Number of iterations (default: 1000)\n";
    std::cout << "  -f, --file FILE          File for read tests (default: test.txt)\n";
    std::cout << "  -m, --mitigations LIST   Comma-separated mitigations (e.g., lfence,cache,all,none)\n";
    std::cout << "  -o, --output FILE        Output CSV file\n";
    std::cout << "  -s, --setup              Create sealed test files\n";
    std::cout << "  -h, --help               Show this help\n";
}

int main(int argc, char* argv[]) {
    std::string test_type;
    int iterations = 1000;
    std::string filename = "test.txt";
    std::string output_file;
    std::string mitigations = "none";
    bool setup_files = false;

    static struct option long_options[] = {
        {"test", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"file", required_argument, 0, 'f'},
        {"mitigations", required_argument, 0, 'm'},
        {"output", required_argument, 0, 'o'},
        {"setup", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "t:i:f:m:o:sh", long_options, nullptr)) != -1) {
        switch (opt) {
            case 't': test_type = optarg; break;
            case 'i': iterations = std::stoi(optarg); break;
            case 'f': filename = optarg; break;
            case 'm': mitigations = optarg; break;
            case 'o': output_file = optarg; break;
            case 's': setup_files = true; break;
            case 'h': print_usage(argv[0]); return 0;
            default: print_usage(argv[0]); return 1;
        }
    }

    parse_mitigations(mitigations);
    print_config();

    if (initialize_enclave() < 0) {
        std::cerr << "Failed to initialize enclave\n";
        return 1;
    }

    BenchmarkRunner runner;
    runner.setup_environment();

    if (setup_files) {
        std::cout << "Creating sealed test files..." << std::endl;
        runner.create_sealed_test_file(filename);
        sgx_destroy_enclave(global_eid);
        return 0;
    }

    if (test_type.empty()) {
        std::cerr << "Error: Test type required\n";
        print_usage(argv[0]);
        sgx_destroy_enclave(global_eid);
        return 1;
    }

    // Warm-up
    std::cout << "Warming up CPU..." << std::endl;
    for (int i = 0; i < 200; ++i) {
         ecall_warmup(global_eid);
    }
    std::cout << "Warm-up complete. Starting benchmark." << std::endl;

    BenchmarkResult result = {0.0, 0, 0.0};
    if (test_type == "ecall") {
        result = runner.benchmark_empty_ecall(iterations);
    } else if (test_type == "pure_ocall") {
        result = runner.benchmark_pure_ocall(iterations);
    } else if (test_type == "pingpong") {
        result = runner.benchmark_ping_pong(iterations);
    } else if (test_type == "untrusted_file") {
        result = runner.benchmark_file_read(filename, iterations);
    } else if (test_type == "sealed_file") {
        std::string sealed_filename = filename + ".sealed";
        result = runner.benchmark_sgx_file_read(sealed_filename, iterations);
    } else if (test_type == "crypto") {
        result = runner.benchmark_crypto_workload(iterations);
    } else {
        std::cerr << "Unknown test type: " << test_type << "\n";
        sgx_destroy_enclave(global_eid);
        return 1;
    }

    double time_per_op = (result.time_ms * 1000.0) / iterations;
    std::cout << "Results: " << result.time_ms << "ms total, " << time_per_op << "μs per operation, "
              << result.cycles_per_op << " cycles per operation\n";

    if (!output_file.empty()) {
        std::ofstream csv(output_file, std::ios::app);
        csv << test_type << "," << mitigations << ","
            << iterations << "," << result.time_ms << "," << time_per_op << ","
            << result.cycles << "," << result.cycles_per_op << "\n";
    }

    sgx_destroy_enclave(global_eid);
    return 0;
}
