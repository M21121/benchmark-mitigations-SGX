# SGX Mitigation Performance Benchmark

## Performance Results (Cycles per Operation)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 7435           | 7591           | 8826           | 7894           | 8141           | 7511           | 7556           |
| **pure_ocall**   | 6531           | 7784           | 7219           | 7133           | 6961           | 6746           | 6650           |
| **pingpong**     | 14400          | 14340          | 14276          | 14641          | 14473          | 14889          | 16583          |
| **untrusted_file** | 49091          | 49717          | 50369          | 77719          | 50143          | 47814          | 77918          |
| **sealed_file**  | 40864          | 40469          | 39604          | 54610          | 44226          | 40504          | 110480         |
| **crypto**       | 20015          | 20656          | 20642          | 34692          | 22396          | 19666          | 54708          |

## Overhead - Cycles (Percentage)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 0.0%           | +2.1%          | +18.7%         | +6.2%          | +9.5%          | +1.0%          | +1.6%          |
| **pure_ocall**   | 0.0%           | +19.2%         | +10.5%         | +9.2%          | +6.6%          | +3.3%          | +1.8%          |
| **pingpong**     | 0.0%           | -0.4%          | -0.9%          | +1.7%          | +0.5%          | +3.4%          | +15.2%         |
| **untrusted_file** | 0.0%           | +1.3%          | +2.6%          | +58.3%         | +2.1%          | -2.6%          | +58.7%         |
| **sealed_file**  | 0.0%           | -1.0%          | -3.1%          | +33.6%         | +8.2%          | -0.9%          | +170.4%        |
| **crypto**       | 0.0%           | +3.2%          | +3.1%          | +73.3%         | +11.9%         | -1.7%          | +173.3%        |

## Performance Results (Time per Operation in μs)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 3.54013        | 3.61445        | 4.20268        | 3.75906        | 3.87629        | 3.57644        | 3.59798        |
| **pure_ocall**   | 3.10987        | 3.7063         | 3.43737        | 3.3966         | 3.31433        | 3.21212        | 3.1665         |
| **pingpong**     | 6.85701        | 6.8285         | 6.79795        | 6.97175        | 6.89183        | 7.08963        | 7.8966         |
| **untrusted_file** | 23.3764        | 23.6747        | 23.985         | 37.0088        | 23.8772        | 22.7685        | 37.1037        |
| **sealed_file**  | 19.4588        | 19.2706        | 18.8587        | 26.0044        | 21.06          | 19.2875        | 52.6091        |
| **crypto**       | 9.53091        | 9.83609        | 9.82944        | 16.5198        | 10.6648        | 9.3645         | 26.051         |

## Overhead - Time (Percentage)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 0.0%           | +2.1%          | +18.7%         | +6.2%          | +9.5%          | +1.0%          | +1.6%          |
| **pure_ocall**   | 0.0%           | +19.2%         | +10.5%         | +9.2%          | +6.6%          | +3.3%          | +1.8%          |
| **pingpong**     | 0.0%           | -0.4%          | -0.9%          | +1.7%          | +0.5%          | +3.4%          | +15.2%         |
| **untrusted_file** | 0.0%           | +1.3%          | +2.6%          | +58.3%         | +2.1%          | -2.6%          | +58.7%         |
| **sealed_file**  | 0.0%           | -1.0%          | -3.1%          | +33.6%         | +8.2%          | -0.9%          | +170.4%        |
| **crypto**       | 0.0%           | +3.2%          | +3.1%          | +73.3%         | +11.9%         | -1.7%          | +173.3%        |

## SGX Test Workload Summary

| Test               | Description                              | Workload                                                             |
| ------------------ | ---------------------------------------- | -------------------------------------------------------------------- |
| **ecall**          | Minimal ECALL operation                  | ECALL with 100 arithmetic operations                               |
| **pure_ocall**     | Isolated OCALL time                     | ECALL → setup → measure N OCALLs (each with 100 ops) → return       |
| **pingpong**       | ECALL that calls OCALL and returns      | ECALL with parameter → OCALL with 100 ops → return                   |
| **untrusted_file** | File I/O via OCALL with basic processing | ECALL → read 8KB file via OCALL → checksum data |
| **sealed_file**    | SGX sealed file I/O with hardware crypto | ECALL → read sealed file → SGX unseal → checksum   |
| **crypto**         | Cryptographic workload simulation        | Hash computation + key derivation on 4KB data with periodic barriers |

## Mitigation Explanations

| Mitigation          | Purpose                        | How It Works                                                                                                                                           |
| ------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **lfence**          | Prevent speculative loads      | CPU instruction that blocks load operations until all prior instructions complete                        |
| **mfence**          | Prevent speculative memory ops | CPU instruction that blocks all memory operations until prior instructions complete                     |
| **cache**           | Prevent cache-based attacks    | Flushes memory from CPU cache using `clflush` instruction on 64-byte cache lines                                                                      |
| **constant**        | Prevent data-dependent timing  | Uses constant-time algorithms for memory operations with secure cleanup and cache flushing                                                             |
| **memory**          | Enforce memory ordering        | Uses `mfence` to prevent memory reordering attacks                                                        |
