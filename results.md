# SGX Mitigation Performance Benchmark

## Performance Results (Time per Operation in μs)

| Operation    | none   | lfence | mfence | cpuid  | all_speculation | cache  | timing | constant | memory | hyperthreading | all    |
| ------------ | ------ | ------ | ------ | ------ | --------------- | ------ | ------ | -------- | ------ | -------------- | ------ |
| **ecall**    | 17.102 | 17.504 | 17.030 | 7.156  | 7.129           | 16.939 | 19.037 | 17.158   | 17.143 | 15.884         | 6.378  |
| **ocall**    | 23.992 | 24.014 | 21.933 | 23.916 | 24.427          | 21.889 | 24.042 | 24.048   | 23.808 | 24.980         | 24.821 |
| **pingpong** | 15.296 | 14.363 | 15.739 | 6.307  | 7.545           | 15.488 | 18.125 | 15.359   | 15.516 | 16.872         | 7.873  |
| **fileread** | 41.091 | 40.260 | 41.144 | 7.640  | 7.620           | 74.538 | 42.191 | 40.337   | 41.372 | 40.197         | 6.026  |
| **sgxread**  | 41.108 | 39.744 | 42.096 | 7.661  | 7.457           | 72.600 | 42.876 | 36.892   | 40.660 | 37.671         | 5.842  |
| **crypto**   | 75.260 | 86.496 | 76.780 | 7.508  | 5.719           | 93.484 | 83.374 | 103.229  | 74.940 | 74.439         | 5.721  |

## SGX Test Workload Summary

| Test           | Description                                | Workload                                                             |
| -------------- | ------------------------------------------ | -------------------------------------------------------------------- |
| **ecall**      | Minimal ECALL operation                    | ECALL with 2000 arithmetic operations and mitigations                |
| **pure_ocall** | Isolated OCALL time                        | ECALL → 2000 ops → OCALL → 100 ops → return                          |
| **pingpong**   | ECALL that calls OCALL and returns         | ECALL with parameter → immediate OCALL back → return                 |
| **fileread**   | File I/O via OCALL with basic processing   | ECALL → read 100KB file via OCALL → checksum data → cleanup          |
| **sgxread**    | File I/O with SGX-specific data processing | Same as fileread + frequent barriers every 64 bytes + secure cleanup |
| **crypto**     | Cryptographic workload simulation          | Hash computation + key derivation on 4KB data with periodic barriers |

## Mitigation Explanations

| Mitigation          | Purpose                        | How It Works                                                                        | Performance Impact        |
| ------------------- | ------------------------------ | ----------------------------------------------------------------------------------- | ------------------------- |
| **lfence**          | Prevent speculative loads      | CPU instruction that blocks load operations until all prior instructions complete   | Minimal (~0.4μs)          |
| **mfence**          | Prevent speculative memory ops | CPU instruction that blocks all memory operations until prior instructions complete | Minimal (~0.3μs)          |
| **cpuid**           | Serialize CPU pipeline         | Heavy CPU instruction that flushes entire execution pipeline and stops speculation  | Anomalous speedup (weird) |
| **all_speculation** | Combined speculation defense   | Executes lfence + mfence + cpuid in sequence for maximum protection                 | ~7μs (cpuid-dominated)    |
| **cache**           | Prevent cache-based attacks    | Flushes memory from CPU cache using `clflush` instruction                           | High (~18-35μs)           |
| **timing**          | Obscure execution timing       | Adds random delay (50-549 iterations) to hide timing patterns                       | Low (~1-8μs)              |
| **constant**        | Prevent data-dependent timing  | Uses constant-time algorithms for memory operations (memcpy, memset)                | Variable (1-28μs)         |
| **memory**          | Enforce memory ordering        | Uses `mfence` to prevent memory reordering attacks                                  | Minimal (~0.3μs)          |
| **hyperthreading**  | Isolate from other threads     | Pins process to physical CPU core to prevent hyperthreading attacks                 | Variable (-1 to +1μs)     |
