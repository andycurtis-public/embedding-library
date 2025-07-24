// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_x86_float_H
#define _embed_x86_float_H

#include <stdint.h>
#include <stddef.h>
#include <immintrin.h> // AVX intrinsics for x86

#if defined(__AVX512F__)

// AVX-512 implementation of dot product (optimized for size divisible by 16)
static inline float dot_product_avx(const float *a, const float *b, size_t size) {
    __m512 sum = _mm512_setzero_ps(); // 512-bit register for accumulating sums

    for (size_t i = 0; i < size; i += 16) {
        __m512 va = _mm512_loadu_ps(a + i); // Load 16 floats from vector a
        __m512 vb = _mm512_loadu_ps(b + i); // Load 16 floats from vector b
        sum = _mm512_add_ps(sum, _mm512_mul_ps(va, vb)); // Multiply and accumulate
    }

    // Horizontal sum of all elements in the register
    return _mm512_reduce_add_ps(sum);
}

#else

// AVX implementation of dot product (fallback for non-AVX512 systems)
static inline float dot_product_avx(const float *a, const float *b, size_t size) {
    __m256 sum = _mm256_setzero_ps(); // 256-bit register for accumulating sums

    for (size_t i = 0; i < size; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i); // Load 8 floats from vector a
        __m256 vb = _mm256_loadu_ps(b + i); // Load 8 floats from vector b
        sum = _mm256_add_ps(sum, _mm256_mul_ps(va, vb)); // Multiply and accumulate
    }

    // Sum the elements in the 256-bit register
    float partial[8];
    _mm256_storeu_ps(partial, sum);
    float result = 0.0f;
    for (int i = 0; i < 8; ++i) {
        result += partial[i];
    }

    return result;
}

#endif

#endif // _embed_x86_float_H