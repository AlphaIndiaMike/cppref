#!/bin/bash
# =============================================================================
# test.sh - Build and run all tests
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/project"
IMAGE_NAME="gtest-dev:latest"

# Build image if it doesn't exist
if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "Image not found. Building..."
    "${SCRIPT_DIR}/build.sh"
fi

echo "Running tests..."
docker run --rm \
    -v "${PROJECT_DIR}:/project" \
    -w /project \
    "${IMAGE_NAME}" \
    bash -c "
        mkdir -p build &&
        cd build &&
        cmake .. &&
        make -j\$(nproc) &&
        ctest --output-on-failure
    "
