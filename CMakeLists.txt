# SPDX-FileCopyrightText: 2024-2025 Knode.ai
# SPDX-License-Identifier: Apache-2.0
# Maintainer: Andy Curtis <contactandyc@gmail.com>

cmake_minimum_required(VERSION 3.20)
project(the_embedding_library VERSION 1.0.0 LANGUAGES C)

# ── Build variant selection (matches your other libs) ──────────────────────────
set(A_BUILD_VARIANT "debug" CACHE STRING "Build variant (debug|memory|coverage|static|shared)")
set_property(CACHE A_BUILD_VARIANT PROPERTY STRINGS debug memory coverage static shared)

# Flavor knobs (same names as your other libs)
set(A_BUILD_MEMORY_DEFINE "_AML_DEBUG_" CACHE STRING "Macro for memory variant")
set(A_BUILD_DEBUG_POSTFIX ""            CACHE STRING "Debug filename postfix")
set(A_BUILD_IMPORT_NAME   ""            CACHE STRING "Exported target name for this flavor")

# SIMD tuning
set(EMBED_SIMD "auto" CACHE STRING "SIMD: auto|avx512|avx2|neon|off")
set_property(CACHE EMBED_SIMD PROPERTY STRINGS auto avx512 avx2 neon off)
option(EMBED_FAST_MATH "Enable fast-math (/fp:fast or -ffast-math where supported)" OFF)

# ── Library sources ────────────────────────────────────────────────────────────
add_library(${PROJECT_NAME}
  src/int8_embedding_table.c
)

# Public headers
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  POSITION_INDEPENDENT_CODE ON
)

# Debug postfix and static name tweak (aligning with your pattern)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND A_BUILD_DEBUG_POSTFIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "${A_BUILD_DEBUG_POSTFIX}")
endif()
if(WIN32 AND NOT BUILD_SHARED_LIBS)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_s")
endif()

# ── Variant-specific compile settings ─────────────────────────────────────────
if(A_BUILD_VARIANT STREQUAL "memory")
  target_compile_definitions(${PROJECT_NAME} PUBLIC ${A_BUILD_MEMORY_DEFINE})
elseif(A_BUILD_VARIANT STREQUAL "coverage")
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(${PROJECT_NAME}    PRIVATE -fprofile-instr-generate)
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g --coverage)
    target_link_options(${PROJECT_NAME}    PRIVATE --coverage)
  endif()
endif()

# ── Optional fast-math (OFF by default) ───────────────────────────────────────
if(EMBED_FAST_MATH)
  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /fp:fast)
  elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -ffast-math)
  endif()
endif()

# ── SIMD flags (portable defaults) ────────────────────────────────────────────
# NOTE:
#   - x86: your int8 AVX path uses AVX2 intrinsics -> enforce AVX2 (not just AVX).
#   - AVX-512 path uses _mm512_maddubs_epi16 -> requires AVX512BW (and AVX512F).
#   - aarch64: NEON is baseline; for 32-bit ARM enable -mfpu=neon if asked.
#
if(EMBED_SIMD STREQUAL "auto")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i.86)$")
    set(EMBED_SIMD x86_auto)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
    set(EMBED_SIMD neon)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(EMBED_SIMD neon)
  else()
    set(EMBED_SIMD off)
  endif()
endif()

if(EMBED_SIMD STREQUAL "avx512" OR EMBED_SIMD STREQUAL "x86_auto")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$" AND NOT MSVC)
    # Try AVX-512 first (F + BW). If you prefer strict detection, gate behind try_compile.
    if(EMBED_SIMD STREQUAL "avx512")
      target_compile_options(${PROJECT_NAME} PRIVATE -mavx512f -mavx512bw)
    elseif(EMBED_SIMD STREQUAL "x86_auto")
      # Conservative default to AVX2 unless you explicitly ask for avx512.
      target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mfma)
    endif()
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$" AND MSVC)
    # MSVC has /arch:AVX2; no broadly supported /arch:AVX512.
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
  endif()
elseif(EMBED_SIMD STREQUAL "avx2")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i.86)$")
    if(MSVC)
      target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    else()
      target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mfma)
    endif()
  endif()
elseif(EMBED_SIMD STREQUAL "neon")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
    # aarch64: NEON is baseline -> no extra flags.
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    if(NOT MSVC)  # MSVC ARM flags differ; usually not used here.
      target_compile_options(${PROJECT_NAME} PRIVATE -mfpu=neon)
    endif()
  endif()
endif()

# ── libm for sqrt/sqrtf (shared builds need it on Linux) ──────────────────────
find_library(M_LIB m)
if(M_LIB)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${M_LIB})
endif()

# ── Install/export like your other libraries ──────────────────────────────────
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Auto-suffixed exported imported target name: the_embedding_library_<variant>
if(NOT A_BUILD_IMPORT_NAME)
  set(A_BUILD_IMPORT_NAME "${PROJECT_NAME}_${A_BUILD_VARIANT}")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME "${A_BUILD_IMPORT_NAME}")

# Export file uses variant suffix (so you can install multiple variants)
set(A_BUILD_EXPORT_FILE_SUFFIX "${A_BUILD_VARIANT}")
install(EXPORT ${PROJECT_NAME}Targets
  FILE "${PROJECT_NAME}Targets-${A_BUILD_EXPORT_FILE_SUFFIX}.cmake"
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# ── Package config (aggregator) ───────────────────────────────────────────────
set(A_BUILD_VARIANTS "debug;memory;coverage;static;shared")
set(_CFG_IN "${CMAKE_CURRENT_BINARY_DIR}/config.cmake.in")
file(WRITE "${_CFG_IN}" [=[
@PACKAGE_INIT@
function(_@A_BUILD_TARGET_BASENAME@_cfg)
  include(CMakeFindDependencyMacro)
  set(_pkg "@A_BUILD_TARGET_BASENAME@")
  set(_ns  "@A_BUILD_EXPORT_NAMESPACE@")
  set(_deps "@A_BUILD_DEPS@")
  set(_flavors "@A_BUILD_VARIANTS@")

  foreach(_dep IN LISTS _deps)
    if(_dep)
      find_dependency("${_dep}" REQUIRED)
    endif()
  endforeach()

  if(NOT TARGET ${_ns}::${_pkg})
    add_library(${_ns}::${_pkg} INTERFACE IMPORTED)
  endif()

  if(NOT DEFINED A_BUILD_VARIANT)
    set(A_BUILD_VARIANT "debug")
  endif()
  list(FIND _flavors "${A_BUILD_VARIANT}" _idx)
  if(_idx EQUAL -1)
    message(FATAL_ERROR "${_pkg}: variant '${A_BUILD_VARIANT}' not recognized (must be one of: ${_flavors})")
  endif()

  set(_picked "${A_BUILD_VARIANT}")
  set(_dir "${CMAKE_CURRENT_LIST_DIR}")
  set(_f "${_pkg}Targets-${_picked}.cmake")
  if(EXISTS "${_dir}/${_f}")
    include("${_dir}/${_f}")
  else()
    message(FATAL_ERROR "${_pkg}: variant '${_picked}' not installed.")
  endif()

  string(REPLACE "-" "_" _picked_us "${_picked}")
  set(_pick ${_ns}::${_pkg}_${_picked_us})
  if(TARGET "${_pick}")
    target_link_libraries(${_ns}::${_pkg} INTERFACE "${_pick}")
  else()
    message(FATAL_ERROR "${_pkg}: expected target '${_pick}' not defined by export.")
  endif()

  set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
  set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL    Release)
endfunction()
_@A_BUILD_TARGET_BASENAME@_cfg()
]=])

set(A_BUILD_TARGET_BASENAME "${PROJECT_NAME}")
set(A_BUILD_EXPORT_NAMESPACE "${PROJECT_NAME}")
set(A_BUILD_DEPS "")  # no package deps

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${_CFG_IN}"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
