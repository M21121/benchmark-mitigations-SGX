# SGX Mitigation Performance Benchmark Results

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


