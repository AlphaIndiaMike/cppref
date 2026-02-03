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

# Resolve absolute path for source to prevent relative path errors
SOURCE_DB_PATH="${SCRIPT_DIR}/../../../90_vendored/database/sqlite3"

# Destination relative to where the script is executed (CWD)
# This matches the Docker volume mount logic (-v $(pwd):/project)
DEST_PARENT_DIR="./integration"
DEST_DB_PATH="${DEST_PARENT_DIR}/sqlite3"

# Resolve absolute path for Interface Contracts
I_HPP_PATH="${SCRIPT_DIR}/../../database/inc/database_connector.h"
C_HPP_PATH="${SCRIPT_DIR}/../../database/inc/sqlite3_database_connector.h"
C_CPP_PATH="${SCRIPT_DIR}/../../database/src/sqlite3_database_connector.cc"

# -----------------------------------------------------------------------------
# Setup & Cleanup
# -----------------------------------------------------------------------------

# 1. Validation: Ensure source exists
if [[ ! -d "${SOURCE_DB_PATH}" ]]; then
    echo "Error: Source database not found at: ${SOURCE_DB_PATH}"
    exit 1
fi

# 2.a Setup: Copy folder
echo "📂 Copying database to integration folder..."
cp -r "${SOURCE_DB_PATH}" "${DEST_PARENT_DIR}/"

# 2.b Setup: Copy interface contracts
echo "📂 Copying required header files..."
cp -r "${I_HPP_PATH}" "${DEST_PARENT_DIR}/"
cp -r "${C_HPP_PATH}" "${DEST_PARENT_DIR}/"
cp -r "${C_CPP_PATH}" "${DEST_PARENT_DIR}/"

# 3. Cleanup: Define function to remove the folder on exit
cleanup() {
    echo "🧹 Cleaning up integration artifacts..."
    # Only remove the specific 'database' folder we copied, not the whole integration dir
    rm -rf "${DEST_DB_PATH}"
cleanup() {
    echo "🧹 Cleaning up integration artifacts..."
    # Only remove the specific 'database' folder we copied, not the whole integration dir
    rm -rf "${DEST_DB_PATH}"
    rm -rf "${DEST_PARENT_DIR}/database_connector.h"
    rm -rf "${DEST_PARENT_DIR}/sqlite3_database_connector.h"
    rm -rf "${DEST_PARENT_DIR}/sqlite3_database_connector.cc"
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
