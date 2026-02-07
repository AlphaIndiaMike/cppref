#!/bin/bash
# =============================================================================
# test.sh - Build and run all tests
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="gtest-dev:latest"

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
SOURCE_NET_PATH="${SCRIPT_DIR}/../../../90_vendored/network/"

# Destination relative to where the script is executed (CWD)
# This matches the Docker volume mount logic (-v $(pwd):/project)
DEST_PARENT_DIR="./integration"
DEST_NET_PATH="${DEST_PARENT_DIR}/network"

# Resolve absolute path for Interface Contracts
I_HPP_PATH="${SCRIPT_DIR}/../../internet/inc/network_connector.h"
C_HPP_PATH="${SCRIPT_DIR}/../../internet/inc/httplib_network_connector.h"
C_CPP_PATH="${SCRIPT_DIR}/../../internet/src/httplib_network_connector.cc"

# -----------------------------------------------------------------------------
# Setup & Cleanup
# -----------------------------------------------------------------------------

# 1. Validation: Ensure source exists
if [[ ! -d "${SOURCE_NET_PATH}" ]]; then
    echo "Error: Source deps not found at: ${SOURCE_NET_PATH}"
    exit 1
fi

# 2.a Setup: Copy folder
echo "📂 Copying deps to integration folder..."
cp -r "${SOURCE_NET_PATH}" "${DEST_PARENT_DIR}/"

# 2.b Setup: Copy interface contracts
echo "📂 Copying required header files..."
cp -r "${I_HPP_PATH}" "${DEST_PARENT_DIR}/"
cp -r "${C_HPP_PATH}" "${DEST_PARENT_DIR}/"
cp -r "${C_CPP_PATH}" "${DEST_PARENT_DIR}/"

# 3. Cleanup: Define function to remove the folder on exit
cleanup() {
    echo "🧹 Cleaning up integration artifacts..."
    rm -rf "${DEST_NET_PATH}"
    rm -rf "${DEST_PARENT_DIR}/network_connector.h"
    rm -rf "${DEST_PARENT_DIR}/httplib_network_connector.h"
    rm -rf "${DEST_PARENT_DIR}/httplib_network_connector.cc"
}

# Register the trap to run on EXIT (happens on success, error, or interrupt)
trap cleanup EXIT

# -----------------------------------------------------------------------------
# Main Execution
# -----------------------------------------------------------------------------

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
            --filter '../src/' \
            --exclude '.*/main\.cc' \
            --exclude '.*/test/.*' \
            --exclude-throw-branches \
            --exclude-unreachable-branches
    "

echo ""
echo "Coverage report: $(pwd)/build/coverage.html"
