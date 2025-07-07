# SGX Mitigation Performance Benchmark

## Performance Results (Time per Operation in μs)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 6.60502        | 6.60397        | 6.55655        | 6.56579        | 6.56282        | 6.69297        | 6.60243        |
| **pure_ocall**   | 5.74083        | 5.81291        | 5.83389        | 5.81329        | 5.79906        | 5.85145        | 5.89935        |
| **pingpong**     | 9.55442        | 10.6205        | 7.16157        | 7.08746        | 6.93105        | 10.3558        | 10.0666        |
| **untrusted_file** | 25.2394        | 26.5924        | 28.7013        | 41.772         | 22.7538        | 27.9871        | 41.1265        |
| **sealed_file**  | 13.7896        | 9.22112        | 10.3831        | 20.2141        | 10.1982        | 9.646          | 29.3757        |
| **crypto**       | 9.16822        | 9.54019        | 11.2563        | 24.9739        | 10.3706        | 16.7291        | 26.1889        |

## Overhead (Percentage)

| **Operation**    | **none**       | **lfence**     | **mfence**     | **cache**      | **constant**   | **memory**     | **all**        |
| ---------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- | -------------- |
| **ecall**        | 0.0%           | +-0.0%         | -0.7%          | -0.6%          | -0.6%          | +1.3%          | +-0.0%         |
| **pure_ocall**   | 0.0%           | +1.3%          | +1.6%          | +1.3%          | +1.0%          | +1.9%          | +2.8%          |
| **pingpong**     | 0.0%           | +11.2%         | -25.0%         | -25.8%         | -27.5%         | +8.4%          | +5.4%          |
| **untrusted_file** | 0.0%           | +5.4%          | +13.7%         | +65.5%         | -9.8%          | +10.9%         | +62.9%         |
| **sealed_file**  | 0.0%           | -33.1%         | -24.7%         | +46.6%         | -26.0%         | -30.0%         | +113.0%        |
| **crypto**       | 0.0%           | +4.1%          | +22.8%         | +172.4%        | +13.1%         | +82.5%         | +185.6%        |

## SGX Test Workload Summary

| Test               | Description                              | Workload                                                             |
| ------------------ | ---------------------------------------- | -------------------------------------------------------------------- |
| **ecall**          | Minimal ECALL operation                  | ECALL with 100 arithmetic operations and conditional mitigations     |
| **pure_ocall**     | Isolated OCALL time                     | ECALL → setup → measure N OCALLs (each with 100 ops) → return       |
| **pingpong**       | ECALL that calls OCALL and returns      | ECALL with parameter → OCALL with 100 ops → return                   |
| **untrusted_file** | File I/O via OCALL with basic processing | ECALL → read 8KB file via OCALL → checksum data → conditional cleanup |
| **sealed_file**    | SGX sealed file I/O with hardware crypto | ECALL → read sealed file → SGX unseal → checksum → secure cleanup    |
| **crypto**         | Cryptographic workload simulation        | Hash computation + key derivation on 4KB data with periodic barriers |

## Mitigation Explanations

| Mitigation          | Purpose                        | How It Works                                                                                                                                           |
| ------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **lfence**          | Prevent speculative loads      | CPU instruction that blocks load operations until all prior instructions complete - applied conditionally based on configuration                       |
| **mfence**          | Prevent speculative memory ops | CPU instruction that blocks all memory operations until prior instructions complete - applied conditionally based on configuration                     |
| **cache**           | Prevent cache-based attacks    | Flushes memory from CPU cache using `clflush` instruction on 64-byte cache lines                                                                      |
| **constant**        | Prevent data-dependent timing  | Uses constant-time algorithms for memory operations with secure cleanup and cache flushing                                                             |
| **memory**          | Enforce memory ordering        | Uses `mfence` to prevent memory reordering attacks - applied conditionally in ecall operations                                                        |
