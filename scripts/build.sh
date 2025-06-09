#!/bin/sh

. scripts/env.sh

cmake -DCMAKE_BUILD_TYPE=Debug  -G "Unix Makefiles" -S "$SOURCE_DIR" -B "$DEBUG_BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release  -G "Unix Makefiles" -S "$SOURCE_DIR" -B "$RELEASE_BUILD_DIR"
cmake --build cmake-build-debug --target all -j"$(nproc)"
cmake --build cmake-build-release --target all -j"$(nproc)"