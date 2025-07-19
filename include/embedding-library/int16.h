// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_int16_H
#define _embed_int16_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Quantize a floating-point array to int16_t
static inline
void int16_from_floats(const float *input, size_t num_floats, int16_t *output) {
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
    float scale_factor = 32767.0f / max_abs;

    // Scale and quantize the input
    for (size_t i = 0; i < num_floats; ++i) {
        output[i] = (int16_t)roundf(input[i] * scale_factor);
    }
}

// Dequantize an int16_t array to floating-point
static inline
void int16_to_floats(const int16_t *input, size_t num_floats, float *output, float scale_factor) {
    for (size_t i = 0; i < num_floats; ++i) {
        output[i] = input[i] / scale_factor;
    }
}

#endif // _embed_int16_H