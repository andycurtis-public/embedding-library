# embedding-library

**SIMD-accelerated primitives for working with dense vector embeddings in C.**
Header-only, portable (x86 & ARM), and released under the Apache-2.0 license.

---

## Highlights

| Capability                      | Float32                                          | Int16                                      | Int8                                                           |
| ------------------------------- | ------------------------------------------------ | ------------------------------------------ | -------------------------------------------------------------- |
| Dot product                     | `dot_product()` (AVX-512 / AVX2 / NEON / scalar) | —                                          | `int8_dot_product()` (AVX-512 / AVX2 / NEON / scalar)          |
| Cosine similarity               | `cosine_similarity()`                            | —                                          | `int8_cosine_similarity()`                                     |
| Quantise ←→ de-quantise         | —                                                | `int16_from_floats()`, `int16_to_floats()` | `int8_from_floats()`, `int8_from_int16s()`, `int8_to_floats()` |
| Embedding table (512-dim, int8) | —                                                | —                                          | `int8_embedding_table_*()` incl. serialization                 |

*Auto-dispatch* picks the fastest implementation available at compile time; you only call the generic functions.

---

## Quick start

```c
#include "embedding-library/float.h"
#include "embedding-library/int8.h"

float  a[512], b[512];           // fill with your vectors
float  cos_f32   = cosine_similarity(a, b, 512);

int8_t qa[512], qb[512];
int8_from_floats(a, 512, qa);
int8_from_floats(b, 512, qb);

float norm_qa = sqrtf(int8_dot_product(qa, qa, 512));
float norm_qb = sqrtf(int8_dot_product(qb, qb, 512));
float cos_i8  = int8_cosine_similarity(qa, norm_qa, qb, norm_qb, 512);
```

### Build

```bash
# GCC / Clang
cc -std=c11 -O3 -mavx2 demo.c -o demo          # x86-64 w/ AVX2
cc -std=c11 -O3 -march=armv8.2-a+simd demo.c   # ARM w/ NEON
```

The library is **header-only**; just add `include/` to your compiler’s search path.

---

## Embedding tables

Efficiently store many 512-dim int8 embeddings:

```c
#include "embedding-library/int8_embedding_table.h"

int8_embedding_table_t *tbl = int8_embedding_table_init(/*rows*/1024);

int8_t vec[512];   double norm = …;             // prepare vector + its L2 norm
int64_t idx = int8_embedding_table_add_embedding(tbl, vec, norm);

double sim = int8_embedding_table_cosine_similarity(tbl, idx, 42);
int8_embedding_table_serialize(tbl, "tbl.bin");

int8_embedding_table_destroy(tbl);
```

Uses chunky allocation (512-row nodes) for cache locality.

---

## Design notes

* **Portable SIMD** – AVX-512, AVX2 and ARM NEON kernels with scalar fall-backs.
* **Quantisation** – simple symmetric linear scaling; pick `int16` first when range matters, then down-cast to `int8`.
* **Header-only** – ideal for integration into existing C / C++ codebases.

---

## License & attribution

Apache-2.0 © 2024-2025 Knode.ai
Maintainer: Andy Curtis [contactandyc@gmail.com](mailto:contactandyc@gmail.com)
