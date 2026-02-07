#!/bin/bash
# =============================================================================
# coverage.sh - Run tests with code coverage report
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="gtest-dev:latest"

# Build image if it doesn't exist
if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "Image not found. Building..."
    "${SCRIPT_DIR}/build.sh"
fi

echo "Running tests with coverage..."
docker run --rm \
    -v "$(pwd):/project" \
    -w /project \
    "${IMAGE_NAME}" \
    bash -c "
        mkdir -p build &&
        cd build &&
        cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON .. &&
        make -j\$(nproc) &&
        ctest --output-on-failure &&
        gcovr -r .. \
            --html --html-details -o coverage.html \
            --exclude '.*/main\.cc' \
            --exclude '.*/test/.*' \
            --exclude-throw-branches \
            --exclude-unreachable-branches
    "

echo ""
echo "Coverage report: $(pwd)/build/coverage.html"
