// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_float_H
#define _embed_float_H

#if defined(__AVX512F__)
#include "embedding-library/x86/float.h"
#elif defined(__AVX__)
#include "embedding-library/x86/float.h"
#elif defined(__ARM_NEON)
#include "embedding-library/arm/float.h"
#else
#include "embedding-library/fallback/float.h"
#endif

#include <stdint.h>
#include <stddef.h>
#include <math.h>

// Generic dot product
static inline float dot_product(const float *a, const float *b, size_t size) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    return dot_product_avx(a, b, size);
#elif defined(__aarch64__) || defined(__arm__)
    return dot_product_neon(a, b, size);
#else
    return dot_product_scalar(a, b, size);
#endif
}

// Compute cosine similarity
static inline float cosine_similarity(const float *a, const float *b, size_t size) {
    float dot = dot_product(a, b, size);
    float norm_a = sqrtf(dot_product(a, a, size));
    float norm_b = sqrtf(dot_product(b, b, size));
    return (norm_a > 0 && norm_b > 0) ? (dot / (norm_a * norm_b)) : 0.0f;
}

#endif // _embed_float_H