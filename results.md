# SGX Mitigation Performance Benchmark

## Performance Results (Time per Operation in μs)

| Operation    | none   | lfence | mfence | cpuid  | all_speculation | cache  | timing | constant | memory | hyperthreading | all    |
| ------------ | ------ | ------ | ------ | ------ | --------------- | ------ | ------ | -------- | ------ | -------------- | ------ |
| **ecall**    | 16.978 | 16.949 | 16.756 | 7.787  | 7.481           | 16.665 | 18.077 | 16.519   | 17.101 | 17.107         | 6.747  |
| **ocall**    | 23.857 | 23.997 | 20.797 | 23.718 | 23.94           | 23.946 | 21.724 | 20.699   | 24.021 | 23.697         | 25.338 |
| **pingpong** | 12.828 | 15.239 | 15.279 | 7.52   | 7.498           | 14.175 | 18.225 | 13.513   | 15.079 | 14.741         | 6.69   |
| **fileread** | 44.33  | 46.632 | 45.893 | 7.676  | 7.66            | 76.72  | 43.67  | 45.758   | 41.831 | 40.475         | 5.945  |
| **sgxread**  | 45.101 | 42.816 | 44.414 | 7.642  | 7.636           | 79.582 | 47.318 | 44.09    | 40.707 | 46.398         | 5.882  |

## SGX Test Workload Summary

| Test           | Description                                | Workload                                                             |
| -------------- | ------------------------------------------ | -------------------------------------------------------------------- |
| **ecall**      | Minimal ECALL operation                    | ECALL with 2000 arithmetic operations and mitigations                |
| **pure_ocall** | Isolated OCALL time                        | ECALL → 2000 ops → OCALL → 100 ops → return                          |
| **pingpong**   | ECALL that calls OCALL and returns         | ECALL with parameter → immediate OCALL back → return                 |
| **fileread**   | File I/O via OCALL with basic processing   | ECALL → read 100KB file via OCALL → checksum data → cleanup          |
| **sgxread**    | File I/O with SGX-specific data processing | Same as fileread + frequent barriers every 64 bytes + secure cleanup |

## Mitigation Explanations

| Mitigation         | Purpose                        | How It Works                                                                        | Performance Impact        |
| ------------------ | ------------------------------ | ----------------------------------------------------------------------------------- | ------------------------- |
| **lfence**         | Prevent speculative loads      | CPU instruction that blocks load operations until all prior instructions complete   | Minimal (~0.4μs)          |
| **mfence**         | Prevent speculative memory ops | CPU instruction that blocks all memory operations until prior instructions complete | Minimal (~0.3μs)          |
| **cpuid**          | Serialize CPU pipeline         | Heavy CPU instruction that flushes entire execution pipeline and stops speculation  | Anomalous speedup (weird) |
| **speculation**    | Combined speculation defense   | Executes lfence + mfence + cpuid in sequence for maximum protection                 | ~7μs (cpuid-dominated)    |
| **cache**          | Prevent cache-based attacks    | Flushes memory from CPU cache using `clflush` instruction                           | High (~35μs)              |
| **timing**         | Obscure execution timing       | Adds random delay (50-549 iterations) to hide timing patterns                       | Low (~1-2μs)              |
| **constant**       | Prevent data-dependent timing  | Uses constant-time algorithms for memory operations (memcpy, memset)                | Minimal (~1μs)            |
| **memory**         | Enforce memory ordering        | Uses `mfence` to prevent memory reordering attacks                                  | Minimal (~0.3μs)          |
| **hyperthreading** | Isolate from other threads     | Pins process to physical CPU core to prevent hyperthreading attacks                 | None (unsure why)         |
