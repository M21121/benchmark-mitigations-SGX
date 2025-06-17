#!/bin/bash
# benchmark_script.sh

ITERATIONS=1000
OUTPUT="benchmark_results.csv"

echo "test_type,mitigations,iterations,total_time_ms,time_per_op_us" > $OUTPUT

echo "Creating test file..."
dd if=/dev/urandom of=test.txt bs=1024 count=100 2>/dev/null

TESTS=("ecall" "pure_ocall" "pingpong" "fileread" "sgxread" "crypto")

# Updated mitigation sets to include individual speculation barriers
MITIGATION_SETS=(
    "none"
    "lfence"
    "mfence"
    "cpuid"
    "all_speculation"
    "cache"
    "timing"
    "constant"
    "memory"
    "hyperthreading"
    "all"
)

make clean
make SGX_MODE=1

if [ $? -ne 0 ]; then
    echo "✗ Build failed. Aborting benchmark."
    exit 1
fi

echo "✓ Build successful. Starting benchmarks..."

for test in "${TESTS[@]}"; do
    for mitigations in "${MITIGATION_SETS[@]}"; do
        echo "-----------------------------------------------------"
        echo "Running test: '$test' with mitigations: '$mitigations'"

        if ./app -t "$test" -i "$ITERATIONS" -m "$mitigations" -o "$OUTPUT"; then
            echo "✓ Completed"
        else
            echo "✗ FAILED"
        fi
    done
done

echo "Benchmark complete. Results in $OUTPUT"
echo ""
echo "Speculation barrier test summary:"
echo "- lfence: Load fence barrier only"
echo "- mfence: Memory fence barrier only"
echo "- cpuid: CPUID serializing instruction only"
echo "- lfence,mfence: Both load and memory fences"
echo "- lfence,cpuid: Load fence + CPUID"
echo "- mfence,cpuid: Memory fence + CPUID"
echo "- all_speculation: All three barriers (lfence+mfence+cpuid)"
echo "- speculation: Legacy combined barrier (lfence+mfence+cpuid in sequence)"
