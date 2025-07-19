// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#ifndef _embed_int8_embedding_table_H
#define _embed_int8_embedding_table_H

#include "embedding-library/int8.h"
#include <stdint.h>
#include <stddef.h>

struct int8_embedding_node_s {
    int8_t *data;
    double *norms;
    uint32_t size;
};
typedef struct int8_embedding_node_s int8_embedding_node_t;

struct int8_embedding_table_s {
    int8_embedding_node_t **table;
    size_t size;
    size_t index;
};
typedef struct int8_embedding_table_s int8_embedding_table_t;

// returns -1 if norm is 0.0
ssize_t int8_embedding_table_add_embedding(int8_embedding_table_t *t, const int8_t *embedding, double norm);
int8_embedding_table_t *int8_embedding_table_init(size_t size);
void int8_embedding_table_destroy(int8_embedding_table_t *t);

static inline
size_t int8_embedding_table_size(int8_embedding_table_t *t) {
    if (t->index == 0)
        return 0;
    return ((t->index - 1) << 9) + t->table[t->index - 1]->size;
}

static inline
double int8_embedding_table_norm(int8_embedding_table_t *t, size_t index) {
    size_t node_index = index >> 9; // index / 512
    size_t offset = index & 0x1FF;  // index % 512
    if (node_index >= t->index) {   // Check if the node index is out of bounds
        return 0.0f;
    }
    return t->table[node_index]->norms[offset];
}

static inline
int8_t *int8_embedding_table_embedding(int8_embedding_table_t *t, size_t index) {
    size_t node_index = index >> 9; // index / 512
    size_t offset = index & 0x1FF;  // index % 512
    if (node_index >= t->index) {   // Check if the node index is out of bounds
        return NULL;
    }
    return t->table[node_index]->data + (offset * 512); // offset *= 512
}

static inline
double int8_embedding_table_cosine_similarity(int8_embedding_table_t *t,
                                              size_t indexA, size_t indexB) {
    double normA = int8_embedding_table_norm(t, indexA);
    double normB = int8_embedding_table_norm(t, indexB);
    if (normA == 0.0f || normB == 0.0f) {
        return 0.0f;
    }
    double dot_product = int8_dot_product(int8_embedding_table_embedding(t, indexA),
                                          int8_embedding_table_embedding(t, indexB), 512);
    return dot_product / (normA * normB);
}

void int8_embedding_table_serialize(int8_embedding_table_t *t, const char *filename);
int8_embedding_table_t *int8_embedding_table_deserialize(const char *filename);

#endif // _embed_int8_embedding_table_H