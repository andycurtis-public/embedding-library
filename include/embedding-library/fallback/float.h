// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_fallback_float_H
#define _embed_fallback_float_H

static inline float dot_product_scalar(const float *a, const float *b, size_t size) {
    float result = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

#endif // _embed_fallback_float_H
