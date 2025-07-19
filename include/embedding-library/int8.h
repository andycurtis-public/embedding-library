// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_int8_H
#define _embed_int8_H

#if defined(__AVX512F__)
#include "embedding-library/x86/int8.h"
#elif defined(__AVX__)
#include "embedding-library/x86/int8.h"
#elif defined(__ARM_NEON)
#include "embedding-library/arm/int8.h"
#else
#include "embedding-library/fallback/int8.h"
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Quantize a floating-point array to int8_t
static inline
void int8_from_floats(const float *input, size_t num_floats, int8_t *output) {
    // Find the maximum absolute value in the input vector
    float max_abs = 0.0f;
    for (size_t i = 0; i < num_floats; ++i) {
        float abs_val = fabsf(input[i]);
        if (abs_val > max_abs) {
            max_abs = abs_val;
        }
    }

    // Avoid division by zero
    if (max_abs == 0.0f) {
        max_abs = 1.0f; // Default to 1.0f to prevent divide-by-zero
    }

    // Compute the scale factor
    float scale_factor = 127.0f / max_abs;

    // Scale and quantize the input
    for (size_t i = 0; i < num_floats; ++i) {
        output[i] = (int8_t)roundf(input[i] * scale_factor);
    }
}

// assume that the input has already had max_abs computed and applied
static inline
void int8_from_int16s(const int16_t *input, size_t num_values, int8_t *output) {
    const float scale_factor = 127.0f / 32767.0f;

    for (size_t i = 0; i < num_values; ++i) {
        output[i] = (int8_t)roundf(input[i] * scale_factor);
    }
}

// Dequantize an int8_t array to floating-point
static inline
void int8_to_floats(const int8_t *input, size_t num_floats, float *output, float scale_factor) {
    for (size_t i = 0; i < num_floats; ++i) {
        output[i] = input[i] / scale_factor;
    }
}

// Compute the dot product of two int8_t embeddings
static inline
int32_t int8_dot_product(const int8_t *embeddingA, const int8_t *embeddingB, size_t embedding_size) {
#if defined(__AVX512F__)
    return int8_dot_product_avx512(embeddingA, embeddingB, embedding_size);
#elif defined(__AVX__)
    return int8_dot_product_avx(embeddingA, embeddingB, embedding_size);
#elif defined(__ARM_NEON)
    return int8_dot_product_neon(embeddingA, embeddingB, embedding_size);
#else
    return int8_dot_product_scalar(embeddingA, embeddingB, embedding_size);
#endif
}

// Compute cosine similarity between two int8_t embeddings
static inline
float int8_cosine_similarity(const int8_t *embeddingA, float normA,
                             const int8_t *embeddingB, float normB,
                             size_t embedding_size) {
    float denominator = normA * normB;
    if (denominator == 0.0f) {
        return 0.0f;
    }
    int32_t dot_product = int8_dot_product(embeddingA, embeddingB, embedding_size);
    return dot_product / denominator;
}

#endif // _embed_int8_H