#!/bin/bash
# benchmark_script.sh

ITERATIONS=100000
OUTPUT="benchmark_results.csv"

echo "test_type,mitigations,iterations,total_time_ms,time_per_op_us" > $OUTPUT

echo "Creating test file..."
dd if=/dev/urandom of=test.txt bs=1024 count=100 2>/dev/null

TESTS=("ecall" "pure_ocall" "pingpong" "untrusted_file" "sealed_file" "crypto")

MITIGATION_SETS=(
    "none"
    "lfence"
    "mfence"
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
echo "- lfence,mfence: Both load and memory fences"
