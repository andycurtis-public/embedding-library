// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_x86_b8_H
#define _embed_x86_b8_H

#include <immintrin.h> // AVX intrinsics for x86
#include <stdint.h>
#include <stddef.h>

#if defined(__AVX512F__)

// AVX-512 implementation for int8 dot product
static inline int32_t int8_dot_product_avx512(const int8_t *a, const int8_t *b, size_t size) {
    __m512i total = _mm512_setzero_si512(); // Accumulator for sums

    for (size_t i = 0; i < size; i += 64) { // Process 64 elements per iteration
        // Load 64 signed 8-bit integers
        __m512i va = _mm512_loadu_si512((__m512i *)&a[i]);
        __m512i vb = _mm512_loadu_si512((__m512i *)&b[i]);

        // Multiply pairs of int8_t and accumulate in int16_t
        __m512i products = _mm512_maddubs_epi16(va, vb);

        // Horizontally add int16_t products into int32_t
        __m512i sums = _mm512_madd_epi16(products, _mm512_set1_epi16(1));

        // Accumulate into total
        total = _mm512_add_epi32(total, sums);
    }

    // Perform horizontal sum of all elements in the accumulator
    return _mm512_reduce_add_epi32(total);
}

#else

// AVX2 implementation for int8 dot product
static inline int32_t int8_dot_product_avx(const int8_t *a, const int8_t *b, size_t size) {
    __m256i total = _mm256_setzero_si256(); // Accumulator for sums

    for (size_t i = 0; i < size; i += 32) { // Process 32 elements per iteration
        // Load 32 signed 8-bit integers
        __m256i va = _mm256_loadu_si256((__m256i *)&a[i]);
        __m256i vb = _mm256_loadu_si256((__m256i *)&b[i]);

        // Multiply pairs of int8_t and accumulate in int16_t
        __m256i products = _mm256_maddubs_epi16(va, vb);

        // Horizontally add int16_t products into int32_t
        __m256i sums = _mm256_madd_epi16(products, _mm256_set1_epi16(1));

        // Accumulate into total
        total = _mm256_add_epi32(total, sums);
    }

    // Sum the elements in the 256-bit register
    int32_t partial[8];
    _mm256_storeu_si256((__m256i *)partial, total);

    int32_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result += partial[i];
    }

    return result;
}

#endif
#endif // _embed_x86_b8_H