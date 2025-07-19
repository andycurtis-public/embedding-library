// SPDX-FileCopyrightText: 2024-2025 Knode.ai
// SPDX-License-Identifier: Apache-2.0
// Maintainer: Andy Curtis <contactandyc@gmail.com>
#include "embedding-library/int8_embedding_table.h"
#include "embedding-library/int8.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Add an embedding to the table
ssize_t int8_embedding_table_add_embedding(int8_embedding_table_t *t, const int8_t *embedding,
                                           double norm) {
    if(norm < 0.0) {
        // compute again
        uint32_t dot_product = int8_dot_product(embedding, embedding, 512);
        if(!dot_product)
            return -1;
        double dp = dot_product;
        norm = sqrt(dp);
    }
    if(norm == 0.0)
        return -1;

    if (t->index > 0) {
        int8_embedding_node_t *n = t->table[t->index - 1];
        if (n->size < 512) {
            int8_t *data = n->data + (n->size * 512); // Move to the next embedding slot
            memcpy(data, embedding, 512); // copies 512 bytes from embedding to data
            n->norms[n->size] = norm;
            n->size++;
            return (((t->index - 1) << 9) + n->size) - 1;
        }
    }
    double *norms = NULL;
    int8_t *data = NULL;
    posix_memalign((void **)&norms, 64, 512 * (512 + sizeof(double)));
    data = (int8_t *)(norms + 512);
    int8_embedding_node_t *n = (int8_embedding_node_t *)malloc(sizeof(int8_embedding_node_t));
    n->data = data;
    n->norms = norms;
    memcpy(data, embedding, 512);
    n->norms[0] = norm;
    t->table[t->index] = n;
    n->size = 1;
    t->index++;
    return (((t->index - 1) << 9) + n->size) - 1;
}


int8_embedding_table_t *int8_embedding_table_init(size_t size) {
    if (size == 0)
        size = 1024*512; // Default size supporting ~268 million embeddings
    int8_embedding_table_t *t = (int8_embedding_table_t *)malloc(sizeof(int8_embedding_table_t));
    t->table = (int8_embedding_node_t **)malloc(size * sizeof(int8_embedding_node_t *));
    t->size = size;
    t->index = 0;
    return t;
}

void int8_embedding_table_destroy(int8_embedding_table_t *t) {
    for (size_t i = 0; i < t->index; i++) {
        free(t->table[i]->norms);
        free(t->table[i]);
    }
    free(t->table);
    free(t);
}

void int8_embedding_table_serialize(int8_embedding_table_t *t, const char *filename) {
    FILE *file = fopen(filename, "ab+");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Determine the current file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const size_t record_size = sizeof(double) + 512;
    size_t existing_records = (file_size >= 0) ? file_size / record_size : 0;

    size_t total_records = int8_embedding_table_size(t);

    // Truncate file if necessary
    if (file_size < 0 || existing_records > total_records) {
        fclose(file);
        file = fopen(filename, "wb");
        if (!file) {
            perror("Failed to reopen file for truncation");
            return;
        }
        existing_records = 0;
    }

    // Serialize only new records
    for (size_t i = existing_records; i < total_records; i++) {
        double norm = int8_embedding_table_norm(t, i);
        fwrite(&norm, sizeof(double), 1, file);
        fwrite(int8_embedding_table_embedding(t, i), sizeof(int8_t), 512, file);
    }

    fclose(file);
}

int8_embedding_table_t *int8_embedding_table_deserialize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    const size_t record_size = sizeof(double) + 512;
    size_t num_records = file_size / record_size;

    int8_embedding_table_t *table = int8_embedding_table_init(0);

    for (size_t i = 0; i < num_records; i++) {
        size_t node_index = i >> 9;
        size_t offset = i & 0x1FF;

        if (node_index >= table->index) {
            int8_embedding_node_t *node = (int8_embedding_node_t *)malloc(sizeof(int8_embedding_node_t));
            posix_memalign((void **)&node->norms, 64, 512 * (512 + sizeof(double)));
            node->data = (int8_t *)(node->norms + 512);
            node->size = 0;
            table->table[table->index++] = node;
        }

        int8_embedding_node_t *node = table->table[node_index];

        fread(&node->norms[offset], sizeof(double), 1, file);
        fread(node->data + (offset * 512), sizeof(int8_t), 512, file);

        node->size++;
    }

    fclose(file);
    return table;
}