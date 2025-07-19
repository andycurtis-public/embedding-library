// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_fallback_b8_H
#define _embed_fallback_b8_H

static inline int32_t int8_dot_product_scalar(const int8_t *a, const int8_t *b, size_t size) {
    int32_t result = 0;
    for (size_t i = 0; i < size; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

#endif
