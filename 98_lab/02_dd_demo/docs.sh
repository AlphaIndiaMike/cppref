#!/bin/bash
# =============================================================================
# docs.sh - Generate all documentation (UML + Doxygen)
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

echo "Generating documentation..."
docker run --rm \
    -v "${PROJECT_DIR}:/project" \
    -w /project \
    "${IMAGE_NAME}" \
    generate-docs

echo ""
echo "Documentation generated:"
echo "  HTML: ${PROJECT_DIR}/docs/html/index.html"
echo "  XML:  ${PROJECT_DIR}/docs/xml/"
echo "  UML:  ${PROJECT_DIR}/docs/uml/"
