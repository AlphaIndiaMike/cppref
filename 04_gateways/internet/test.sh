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

SOURCE_HTTPLIB_PATH="${SCRIPT_DIR}/../../90_vendored/network/httplib"

# Destination relative to where the script is executed (CWD)
# This matches the Docker volume mount logic (-v $(pwd):/project)
DEST_PARENT_DIR="./integration"
DEST_DB_PATH="${DEST_PARENT_DIR}/httplib"

# -----------------------------------------------------------------------------
# Setup & Cleanup
# -----------------------------------------------------------------------------

# 1. Validation: Ensure source exists
if [[ ! -d "${SOURCE_HTTPLIB_PATH}" ]]; then
    echo "Error: Source dependencies not found at: ${SOURCE_HTTPLIB_PATH}"
    exit 1
fi

# 2. Setup: Copy folder
echo "📂 Copying dependencies to integration folder..."
cp -r "${SOURCE_HTTPLIB_PATH}" "${DEST_PARENT_DIR}/"

# 3. Cleanup: Define function to remove the folder on exit
cleanup() {
    echo "🧹 Cleaning up integration artifacts..."
    rm -rf "${DEST_DB_PATH}"
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

echo "Running tests..."
docker run --rm \
    -v "$(pwd):/project" \
    -w /project \
    "${IMAGE_NAME}" \
    bash -c "
        mkdir -p build &&
        cd build &&
        cmake .. &&
        make -j\$(nproc) &&
        ctest --output-on-failure
    "
