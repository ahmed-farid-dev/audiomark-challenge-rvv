# RISC-V Audiomark Challenge: Q15 AXPY Optimization

## Overview
This repository contains a RISC-V Vector (RVV 1.0) implementation of the Q15 Fixed-Point AXPY kernel (`sat_q15(a[i] + alpha * b[i])`). The solution is designed to be **Vector Length Agnostic (VLA)** and highly optimized for performance using specific DSP vector instructions.

## 1. Design Choices

### A. Vector Configuration (LMUL = 8)
I selected an **LMUL (Register Grouping) of m8**.
- **Reasoning:** The AXPY kernel is memory-bound with low register pressure. Using `m8` maximizes the number of elements processed per instruction (up to 64 elements per cycle on VLEN=128 hardware). This significantly amortizes the overhead of the `vsetvl` instruction and loop branching.

### B. Vector Length Agnostic (VLA)
I utilized a strip-mining loop driven by the `vsetvl` instruction.
- **Reasoning:** This ensures portability. The code runs unmodified on any RISC-V Vector hardware, dynamically adapting to the available VLEN (from embedded 64-bit implementations to 512-bit accelerators).

### C. Q15 Arithmetic & Rounding Mode
I utilized the `vsmul.vx` and `vsadd.vv` instructions with explicit Rounding Mode control.
- **Reasoning:**
  - **Saturation:** `vsmul` and `vsadd` handle saturation natively, clamping results to the `int16` range automatically.
  - **Rounding:** I explicitly set the Rounding Mode to **Round-to-Nearest-Up (RNU)** via the intrinsic argument `__RISCV_VXRM_RNU`. This matches standard Audio DSP requirements (`+ 0x4000`) and prevents precision loss compared to truncation.

## 2. Performance Analysis (Theoretical)

Since physical hardware was not available for this implementation, I performed a static instruction scheduling analysis based on a standard VLEN=128 dual-issue pipeline with chaining enabled.

### Methodology
1.  **Scalar Baseline:** ~10 cycles per element (Load, Load, Mul, Shift, Add, Sat, Store, Loop Overhead).
2.  **Vector Potential:**
    -   **Batch Size:** With `LMUL=8`, one instruction processes 64 elements (8 registers * 8 elements/reg).
    -   **Execution Time:** Assuming 128-bit ALU width (8 elements/cycle), execution takes 8 cycles.
    -   **Cost per Element:** ~0.125 cycles per element (compute bound).
3.  **Memory Bottleneck:** The kernel is memory-bound (48 bits/element). On a standard 128-bit bus, the theoretical limit is ~0.4 cycles per element.

### Conclusion
Accounting for real-world memory latency and bus contention, the vectorized solution is expected to achieve a **10x - 20x speedup** over the scalar baseline.
