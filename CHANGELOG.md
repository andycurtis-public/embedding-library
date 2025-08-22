## 08/22/2025

**Modernize Embedding Library build system, licensing, and SIMD backends**

---

## Summary

This PR refactors the **Embedding Library** with a modern multi-variant CMake build, unified tooling, updated SPDX/licensing headers, SIMD backend cleanup, and a refreshed test/coverage setup.

---

## Key Changes

### 🔧 Build & Tooling

* **Removed** legacy files:

  * `CHANGELOG.md`, `build_install.sh`.
* **Added** `build.sh`:

  * Commands: `build`, `install`, `coverage`, `clean`.
  * Coverage integration with `llvm-cov`.
* `.gitignore`: added `build-unix_makefiles`, `build-cov`, `build-coverage`.
* **BUILDING.md**:

  * Updated for *Embedding Library v0.0.1*.
  * Clear local build + install flow.
  * Dependencies:

    * system `libm` only (via package manager).
  * Dockerfile updated for self-contained builds.

### 📦 CMake

* Raised minimum version to **3.20**.
* Project renamed to `embedding_library` (underscore convention).
* **Multi-variant builds**:

  * `debug`, `memory`, `static`, `shared`.
  * Umbrella alias: `embedding_library::embedding_library`.
* Coverage toggle (`A_ENABLE_COVERAGE`) and memory profiling define (`_AML_DEBUG_`).
* Dependency: system math library (`m`).
* Proper **install/export**:

  * Generates `embedding_libraryConfig.cmake` + version file.
  * Namespace: `embedding_library::`.

### 📖 Documentation

* **AUTHORS**: updated Andy Curtis entry with GitHub profile.
* **NOTICE**:

  * Simplified attribution:

    * Andy Curtis (2025), Knode.ai (2024–2025).
  * Removed inline technical contact note.
* Dockerfile:

  * Ubuntu base, configurable CMake version.
  * Non-root `dev` user.
  * Installs only this project (no external repos).

### 📝 Source & Headers

* SPDX headers standardized:

  * En-dash years (`2024–2025`).
  * Andy Curtis explicitly credited.
  * Knode.ai marked with “technical questions” contact.
* Removed redundant `Maintainer:` lines.
* **SIMD backend cleanup**:

  * Unified header guards (`int8`, `int16`, `float`).
  * AVX512/AVX2 and NEON backends now fallback to scalar tails.
  * Clamping added for `int8_from_*` and `int16_from_floats`.
  * Cosine similarity helpers simplified.
* **Embedding table (`int8_embedding_table`)**:

  * Memory allocation hardened with alignment + error checks.
  * Safer serialization/deserialization with truncation handling.
  * Constants (`EMBEDDING_DIM=512`, `NODE_CAPACITY=512`) centralized.
  * Returns `-1` on failure.

### ✅ Tests

* **`tests/CMakeLists.txt`**:

  * Variant-aware build with coverage aggregation.
  * Coverage support for `llvm-profdata` + `llvm-cov`.
* **`tests/build.sh`**:

  * Supports variants (`debug|memory|static|shared|coverage`).
  * Auto job detection for parallel builds.
* No functional test code yet, only infra prepared.

---

## Impact

* 🚀 Streamlined builds with a single script + modern CMake variants.
* 🛡️ Stronger, consistent SPDX headers and NOTICE.
* 📖 Docs clarified with explicit dependency setup.
* 🏎️ SIMD backends now robust with scalar tails + clamping.
* ✅ Safer embedding table I/O + coverage-ready test infra.
