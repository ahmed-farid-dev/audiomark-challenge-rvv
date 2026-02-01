#include <riscv_vector.h>
#include <stdint.h>
#include <stddef.h>

// Function: q15_axpy_rvv
// Calculates: y[i] = sat_q15(a[i] + alpha * b[i])
void q15_axpy_rvv(const int16_t *a, const int16_t *b, int16_t *y, size_t n, int16_t alpha) {
    
    size_t vl;

    // Use a while loop with size_t to prevent optimizer issues
    while (n > 0) {
        // 1. Set Vector Length (vl)
        // Request 16-bit elements (e16) and group 8 registers (m8)
        // The processor returns the number of elements it can process (vl)
        vl = __riscv_vsetvl_e16m8(n);

        // 2. Load Input Vectors
        vint16m8_t va = __riscv_vle16_v_i16m8(a, vl);
        vint16m8_t vb = __riscv_vle16_v_i16m8(b, vl);

        // 3. Saturating Multiply: alpha * b[i]
        // vsmul performs the Q15 multiply: (vb * alpha) >> 15
        // We explicitly set Round-to-Nearest-Up (RNU) to match audio standards.
        vint16m8_t vprod = __riscv_vsmul_vx_i16m8(vb, alpha, __RISCV_VXRM_RNU, vl);

        // 4. Saturating Add: a[i] + prod
        // Adds the vectors and saturates the result to [-32768, 32767]
        vint16m8_t vy = __riscv_vsadd_vv_i16m8(va, vprod, vl);

        // 5. Store Result
        __riscv_vse16_v_i16m8(y, vy, vl);

        // 6. Update Pointers and Count
        a += vl;
        b += vl;
        y += vl;
        n -= vl;
    }
}
