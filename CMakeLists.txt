# SPDX-FileCopyrightText: 2024-2025 Knode.ai
# SPDX-License-Identifier: Apache-2.0
# Maintainer: Andy Curtis <contactandyc@gmail.com>
cmake_minimum_required(VERSION 3.10)

# Project Configuration
project(embedding-library VERSION 0.1.1)

# Variables
set(INCLUDE_DIR_NAME "embedding-library")
set(EXTRA_FILES README.md CHANGELOG.md)

# Source files
file(GLOB SOURCE_FILES src/*.c)

find_package(a-cmake-library REQUIRED)

include(LibraryConfig)
include(LibraryBuild)
