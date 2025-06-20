# SGX Mitigation Performance Benchmark

## Performance Results (Time per Operation in μs)

| **Operation**      | **none** | **lfence** | **mfence** | **cache** | **timing** | **constant** | **memory** | **hyperthreading*** | **all** |
| ------------------ | -------- | ---------- | ---------- | --------- | ---------- | ------------ | ---------- | ------------------- | ------- |
| **ecall**          | 6.66917  | 6.72574    | 6.78912    | 6.67021   | 8.02439    | 6.63059      | 6.91942    | 6.50976             | 7.83275 |
| **pure_ocall**     | 5.9011   | 5.86134    | 5.83041    | 5.93472   | 5.94235    | 5.93486      | 5.86473    | 5.83997             | 5.81651 |
| **pingpong**       | 12.5973  | 12.5876    | 12.8316    | 12.6404   | 15.2482    | 12.9824      | 12.731     | 12.5419             | 14.1735 |
| **untrusted_file** | 22.8575  | 29.6145    | 31.0302    | 63.2539   | 39.2754    | 36.9131      | 36.8176    | 32.944              | 38.043  |
| **sealed_file**    | 16.4631  | 16.6511    | 9.56895    | 36.2543   | 18.9361    | 49.1894      | 16.4896    | 16.291              | 54.2919 |
| **crypto**         | 52.0134  | 51.8776    | 52.4089    | 49.8267   | 53.3002    | 68.2479      | 50.0095    | 42.2501             | 78.9367 |

## Overhead (Percentage)

| **Operation**      | **none** | **lfence** | **mfence** | **cache** | **timing** | **constant** | **memory** | **hyperthreading*** | **all** |
| ------------------ | -------- | ---------- | ---------- | --------- | ---------- | ------------ | ---------- | ------------------- | ------- |
| **ecall**          | 0.0%     | +0.8%      | +1.8%      | +0.0%     | +20.3%     | -0.6%        | +3.8%      | -2.4%               | +17.4%  |
| **pure_ocall**     | 0.0%     | -0.7%      | -1.2%      | +0.6%     | +0.7%      | +0.6%        | -0.6%      | -1.0%               | -1.4%   |
| **pingpong**       | 0.0%     | -0.1%      | +1.9%      | +0.3%     | +21.0%     | +3.1%        | +1.1%      | -0.4%               | +12.5%  |
| **untrusted_file** | 0.0%     | +29.6%     | +35.8%     | +176.7%   | +71.8%     | +61.5%       | +61.1%     | +44.1%              | +66.4%  |
| **sealed_file**    | 0.0%     | +1.1%      | -41.9%     | +120.2%   | +15.0%     | +198.8%      | +0.2%      | -1.0%               | +229.8% |
| **crypto**         | 0.0%     | -0.3%      | +0.8%      | -4.2%     | +2.5%      | +31.2%       | -3.9%      | -18.8%              | +51.8%  |

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
| **timing**          | Obscure execution timing       | Adds random delay (50-549 iterations) using SGX hardware RNG to hide timing patterns                                                                  |
| **constant**        | Prevent data-dependent timing  | Uses constant-time algorithms for memory operations with secure cleanup and cache flushing                                                             |
| **memory**          | Enforce memory ordering        | Uses `mfence` to prevent memory reordering attacks - applied conditionally in ecall operations                                                        |
| **hyperthreading*** | Isolate from other threads     | Pins process to physical CPU core to simulate disabling hyperthreading. Hyperthreading would have to be disabled in the BIOS for a more accurate test. |
