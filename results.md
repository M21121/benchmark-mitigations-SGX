# SGX Mitigation Performance Benchmark

## Performance Results (Cycles per Operation)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 13906          | 13852          | 13944          | 13887          | 13844          | 13933          | 14032          |
| **pure_ocall**   | 12234          | 12213          | 12180          | 11412          | 7907           | 12160          | 6718           |
| **pingpong**     | 14264          | 24890          | 14774          | 21984          | 14809          | 14235          | 14231          |
| **untrusted_file** | 68326          | 45591          | 51921          | 104216         | 46128          | 50844          | 100214         |
| **sealed_file**  | 49798          | 40891          | 46895          | 78911          | 42573          | 39638          | 111507         |
| **crypto**       | 28466          | 19585          | 29871          | 37578          | 33227          | 18940          | 63621          |

## Overhead - Cycles (Percentage)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 0.0%           | -0.4%          | +0.3%          | -0.1%          | -0.4%          | +0.2%          | +0.9%          |
| **pure_ocall**   | 0.0%           | -0.2%          | -0.4%          | -6.7%          | -35.4%         | -0.6%          | -45.1%         |
| **pingpong**     | 0.0%           | +74.5%         | +3.6%          | +54.1%         | +3.8%          | -0.2%          | -0.2%          |
| **untrusted_file** | 0.0%           | -33.3%         | -24.0%         | +52.5%         | -32.5%         | -25.6%         | +46.7%         |
| **sealed_file**  | 0.0%           | -17.9%         | -5.8%          | +58.5%         | -14.5%         | -20.4%         | +123.9%        |
| **crypto**       | 0.0%           | -31.2%         | +4.9%          | +32.0%         | +16.7%         | -33.5%         | +123.5%        |

## Performance Results (Time per Operation in μs)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 6.62164        | 6.59586        | 6.63973        | 6.61263        | 6.59191        | 6.63452        | 6.68144        |
| **pure_ocall**   | 5.82557        | 5.81555        | 5.79974        | 5.43385        | 3.765          | 5.79044        | 3.19907        |
| **pingpong**     | 6.79228        | 11.8524        | 7.03479        | 10.4684        | 7.05191        | 6.77844        | 6.77656        |
| **untrusted_file** | 32.536         | 21.7097        | 24.724         | 49.626         | 21.9653        | 24.2114        | 47.7206        |
| **sealed_file**  | 23.7129        | 19.4716        | 22.3305        | 37.5763        | 20.2726        | 18.8752        | 53.0984        |
| **crypto**       | 13.5552        | 9.32587        | 14.2241        | 17.8939        | 15.8219        | 9.0186         | 30.2955        |

## Overhead - Time (Percentage)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 0.0%           | -0.4%          | +0.3%          | -0.1%          | -0.4%          | +0.2%          | +0.9%          |
| **pure_ocall**   | 0.0%           | -0.2%          | -0.4%          | -6.7%          | -35.4%         | -0.6%          | -45.1%         |
| **pingpong**     | 0.0%           | +74.5%         | +3.6%          | +54.1%         | +3.8%          | -0.2%          | -0.2%          |
| **untrusted_file** | 0.0%           | -33.3%         | -24.0%         | +52.5%         | -32.5%         | -25.6%         | +46.7%         |
| **sealed_file**  | 0.0%           | -17.9%         | -5.8%          | +58.5%         | -14.5%         | -20.4%         | +123.9%        |
| **crypto**       | 0.0%           | -31.2%         | +4.9%          | +32.0%         | +16.7%         | -33.5%         | +123.5%        |

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
