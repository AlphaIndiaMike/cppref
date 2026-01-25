#!/bin/bash
# Build Docker image from vendored dependencies
# This build is fully offline and reproducible

set -e

IMAGE_NAME="cpp-docs"
IMAGE_TAG="vendored"
DOCKERFILE="Dockerfile.vendored"

echo "================================================"
echo "  Building Vendored Docker Image"
echo "================================================"
echo "Image:      $IMAGE_NAME:$IMAGE_TAG"
echo "Dockerfile: $DOCKERFILE"
echo ""

# Check if vendored directory exists
if [ ! -d "vendored" ]; then
    echo "ERROR: vendored/ directory not found!"
    echo ""
    echo "Please run ./vendor.sh first to download dependencies:"
    echo "  ./vendor.sh"
    echo ""
    exit 1
fi

# Verify critical vendored files exist
MISSING=0

echo "Checking vendored dependencies..."

# Check PlantUML
if ls vendored/plantuml/plantuml-*.jar 1> /dev/null 2>&1; then
    echo "  ✓ Found: PlantUML JAR"
else
    echo "  ✗ Missing: PlantUML JAR (vendored/plantuml/plantuml-*.jar)"
    MISSING=$((MISSING + 1))
fi

# Check clang-uml
if ls vendored/clang-uml/clang-uml-*.tar.gz 1> /dev/null 2>&1; then
    echo "  ✓ Found: clang-uml source"
else
    echo "  ✗ Missing: clang-uml source (vendored/clang-uml/clang-uml-*.tar.gz)"
    MISSING=$((MISSING + 1))
fi

# Check apt-packages
if [ -d "vendored/apt-packages" ] && [ "$(ls -A vendored/apt-packages/*.deb 2>/dev/null)" ]; then
    DEB_COUNT=$(ls -1 vendored/apt-packages/*.deb 2>/dev/null | wc -l)
    echo "  ✓ Found: APT packages ($DEB_COUNT .deb files)"
else
    echo "  ✗ Missing: APT packages (vendored/apt-packages/*.deb)"
    MISSING=$((MISSING + 1))
fi

# Check base image
if [ -f "vendored/base-image/ubuntu-22.04.tar" ]; then
    echo "  ✓ Found: Ubuntu base image"
else
    echo "  ✗ Missing: Ubuntu base image (vendored/base-image/ubuntu-22.04.tar)"
    MISSING=$((MISSING + 1))
fi

if [ $MISSING -gt 0 ]; then
    echo ""
    echo "ERROR: $MISSING vendored dependencies missing!"
    echo "Please run ./vendor.sh to download all dependencies."
    exit 1
fi

echo ""
echo "All dependencies verified. Starting build..."
echo ""

# Load vendored base image if it exists
if [ -f "vendored/base-image/ubuntu-22.04.tar" ]; then
    echo "Loading vendored Ubuntu base image..."
    docker load -i vendored/base-image/ubuntu-22.04.tar
    echo "✓ Base image loaded"
    echo ""
fi

# Build the image
echo "Building Docker image..."
docker build \
    -f "$DOCKERFILE" \
    -t "$IMAGE_NAME:$IMAGE_TAG" \
    -t "$IMAGE_NAME:latest" \
    .

echo ""
echo "================================================"
echo "  Build Complete!"
echo "================================================"
echo "Image: $IMAGE_NAME:$IMAGE_TAG"
echo ""
echo "Verify the build:"
echo "  docker images | grep $IMAGE_NAME"
echo ""
echo "Test it:"
echo "  ./docs.sh"
echo ""
echo "Build details:"
docker images --format "table {{.Repository}}:{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}" | grep "$IMAGE_NAME"
echo ""
echo "This build uses ONLY vendored dependencies."
echo "It will produce identical results even years from now."
echo "================================================"
