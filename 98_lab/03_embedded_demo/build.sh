#!/bin/bash
# Build Docker Image for Embedded C Development

set -e

IMAGE_NAME="embedded-c-dev"
MODE="${1:-online}"

if [ "$MODE" = "vendored" ]; then
    echo "================================================"
    echo "  Building VENDORED image                       "
    echo "================================================"

    # Check if vendored directory exists
    if [ ! -d "vendored" ]; then
        echo "ERROR: vendored/ directory not found!"
        echo "Please run ./vendor.sh first to download dependencies."
        exit 1
    fi

    # Verify key vendored files exist
    if [ ! -f "vendored/base-image/ubuntu-22.04.tar" ]; then
        echo "ERROR: Ubuntu base image not found in vendored/"
        echo "Please run ./vendor.sh to download it."
        exit 1
    fi

    # Load vendored base image
    echo "Loading vendored Ubuntu base image..."
    docker load -i vendored/base-image/ubuntu-22.04.tar

    # Build using vendored Dockerfile
    echo "Building from Dockerfile.vendored..."
    docker build -f Dockerfile.vendored -t ${IMAGE_NAME}:vendored .

    echo ""
    echo "✓ Vendored build complete!"
    echo "  Image: ${IMAGE_NAME}:vendored"
    echo ""
    echo "Next steps:"
    echo "  ./test.sh        # Run unit tests"
    echo "  ./coverage.sh    # Generate coverage report"
    echo "  ./docs.sh        # Generate documentation"

elif [ "$MODE" = "online" ] || [ "$MODE" = "" ]; then
    echo "================================================"
    echo "  Building ONLINE image (requires internet)"
    echo "================================================"
    echo ""
    echo "Note: For reproducible/offline builds, use:"
    echo "  ./vendor.sh && ./build.sh vendored"
    echo ""

    docker build -f Dockerfile -t ${IMAGE_NAME}:latest .

    echo ""
    echo "✓ Online build complete!"
    echo "  Image: ${IMAGE_NAME}:latest"
    echo ""
    echo "Next steps:"
    echo "  ./test.sh        # Run unit tests"
    echo "  ./coverage.sh    # Generate coverage report"
    echo "  ./docs.sh        # Generate documentation"

else
    echo "Usage: $0 [online|vendored]"
    echo ""
    echo "  online   - Build using internet (default)"
    echo "  vendored - Build using vendored dependencies (offline)"
    exit 1
fi
