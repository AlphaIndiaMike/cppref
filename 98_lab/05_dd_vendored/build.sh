#!/bin/bash
# =============================================================================
# build.sh - Build the Docker image
# =============================================================================
# Usage:
#   ./build.sh           - Online build (downloads from internet)
#   ./build.sh vendored  - Vendored build (offline, reproducible)
#   ./build.sh --help    - Show help
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="cpp-docs"
IMAGE_TAG="latest"
BUILD_MODE="${1:-online}"

show_help() {
    cat << EOF
Build Script for C++ Documentation Generator

Usage:
  ./build.sh [MODE]

Modes:
  online     Build using internet dependencies (default, less reproducible)
  vendored   Build using vendored dependencies (offline, fully reproducible)
  --help     Show this help

Recommended: Use 'vendored' mode for production and long-term reproducibility.

Examples:
  ./build.sh vendored    # Reproducible build for production
  ./build.sh             # Quick build for development

Prerequisites for vendored build:
  Run ./vendor.sh first to download all dependencies

EOF
    exit 0
}

# Parse arguments
if [ "$BUILD_MODE" = "--help" ] || [ "$BUILD_MODE" = "-h" ]; then
    show_help
fi

if [ "$BUILD_MODE" = "vendored" ]; then
    echo "================================================"
    echo "  Building with VENDORED dependencies"
    echo "================================================"
    echo "This build is fully reproducible and offline."
    echo ""
    
    if [ ! -d "vendored" ]; then
        echo "ERROR: vendored/ directory not found!"
        echo ""
        echo "Please run ./vendor.sh first:"
        echo "  ./vendor.sh"
        echo ""
        exit 1
    fi
    
    exec ./build-vendored.sh
else
    echo "================================================"
    echo "  Building with ONLINE dependencies"
    echo "================================================"
    echo "⚠️  Warning: This build downloads from the internet."
    echo "⚠️  For reproducible builds, use: ./build.sh vendored"
    echo ""
    echo "Building Docker image: ${IMAGE_NAME}:${IMAGE_TAG}"
    
    docker build -t "${IMAGE_NAME}:${IMAGE_TAG}" "${SCRIPT_DIR}"
    
    echo ""
    echo "Done! Image ready: ${IMAGE_NAME}:${IMAGE_TAG}"
    echo ""
    echo "💡 Tip: For production, consider vendored builds:"
    echo "   ./vendor.sh          # Download dependencies once"
    echo "   ./build.sh vendored  # Build offline forever"
fi
