// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_arm_b8_H
#define _embed_arm_b8_H

#include <arm_neon.h>
#include <stdint.h>
#include <stddef.h>

// Compute dot product directly for signed 8-bit embeddings (b8)
static inline
int32_t int8_dot_product_neon(const int8_t *embeddingA, const int8_t *embeddingB, size_t embedding_size) {
    int32x4_t total_sum = vdupq_n_s32(0);  // Initialize a 128-bit accumulator

    for (size_t i = 0; i < embedding_size; i += 16) {  // Process 16 elements at a time
        // Load 16 signed 8-bit values from each embedding
        int8x16_t embeddingA_values = vld1q_s8(&embeddingA[i]);
        int8x16_t embeddingB_values = vld1q_s8(&embeddingB[i]);

        // Multiply and accumulate lower 8 bytes (8 elements)
        int16x8_t product_low = vmull_s8(vget_low_s8(embeddingA_values), vget_low_s8(embeddingB_values));

        // Multiply and accumulate upper 8 bytes (8 elements)
        int16x8_t product_high = vmull_s8(vget_high_s8(embeddingA_values), vget_high_s8(embeddingB_values));

        // Accumulate results into the total sum
        total_sum = vaddq_s32(total_sum, vaddl_s16(vget_low_s16(product_low), vget_high_s16(product_low)));
        total_sum = vaddq_s32(total_sum, vaddl_s16(vget_low_s16(product_high), vget_high_s16(product_high)));
    }

    // Perform horizontal sum to get the final result
    int32x2_t sum_pair = vadd_s32(vget_low_s32(total_sum), vget_high_s32(total_sum)); // Add low and high parts
    int32_t final_sum = vget_lane_s32(vpadd_s32(sum_pair, sum_pair), 0); // Add across lanes

    return final_sum;
}

#endif // _embed_arm_b8_H
