#!/bin/bash
# =============================================================================
# shell.sh - Interactive shell in the container
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/project"
IMAGE_NAME="cpp-docs:latest"

# Build image if it doesn't exist
if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "Image not found. Building..."
    "${SCRIPT_DIR}/build.sh"
fi

echo "Starting interactive shell..."
docker run --rm -it \
    -v "${PROJECT_DIR}:/project" \
    -w /project \
    "${IMAGE_NAME}" \
    bash
