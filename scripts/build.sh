#!/bin/sh

. scripts/env.sh

JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc)

cmake -DCMAKE_BUILD_TYPE=Debug  -G "Unix Makefiles" -S "$SOURCE_DIR" -B "$DEBUG_BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release  -G "Unix Makefiles" -S "$SOURCE_DIR" -B "$RELEASE_BUILD_DIR"
cmake --build cmake-build-debug --target all -j"$JOBS"
cmake --build cmake-build-release --target all -j"$JOBS"