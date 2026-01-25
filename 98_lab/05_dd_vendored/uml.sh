#!/bin/bash
# =============================================================================
# uml.sh - Generate UML diagrams only
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

echo "Generating UML diagrams..."
docker run --rm \
    -v "${PROJECT_DIR}:/project" \
    -w /project \
    "${IMAGE_NAME}" \
    bash -c "
        mkdir -p docs/uml &&
        clang-uml --config .clang-uml &&
        plantuml -tsvg docs/uml/*.puml 2>/dev/null || true &&
        plantuml -tpng docs/uml/*.puml 2>/dev/null || true
    "

echo ""
echo "UML diagrams generated: ${PROJECT_DIR}/docs/uml/"
