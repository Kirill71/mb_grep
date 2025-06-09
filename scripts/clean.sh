#!/bin/sh

. scripts/env.sh

if [ -d "$DEBUG_BUILD_DIR" ]; then
    cmake --build "$DEBUG_BUILD_DIR" --target clean
fi

if [ -d "$RELEASE_BUILD_DIR" ]; then
    cmake --build "$RELEASE_BUILD_DIR" --target clean
fi