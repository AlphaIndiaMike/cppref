#!/bin/bash
# Vendor Dependencies Script for Google Test Docker Environment
# Downloads and stores all external dependencies for offline/reproducible builds

set -e

VENDOR_DIR="$(pwd)/vendored"

echo "================================================"
echo "  Vendoring Dependencies"
echo "================================================"
echo "Target: $VENDOR_DIR"
echo ""

# Check prerequisites
if ! command -v docker &> /dev/null; then
    echo "ERROR: docker is not installed. Please install it first."
    exit 1
fi

# Create vendor directory structure
mkdir -p "$VENDOR_DIR"/{base-image,apt-packages}

# ===== 1. Ubuntu Base Image =====
echo "[1/3] Vendoring Ubuntu base image..."
BASE_IMAGE="ubuntu:22.04"
BASE_IMAGE_FILE="$VENDOR_DIR/base-image/ubuntu-22.04.tar"

if [ -f "$BASE_IMAGE_FILE" ]; then
    echo "   ✓ Already vendored: ubuntu-22.04.tar"
else
    echo "   Pulling and saving Ubuntu 22.04 image..."
    docker pull "$BASE_IMAGE"
    docker save "$BASE_IMAGE" -o "$BASE_IMAGE_FILE"
    echo "   ✓ Vendored: ubuntu-22.04.tar ($(du -h "$BASE_IMAGE_FILE" | cut -f1))"
fi

# ===== 2. APT Packages =====
echo ""
echo "[2/3] Vendoring APT packages..."
APT_CACHE_DIR="$VENDOR_DIR/apt-packages"

# Create a temporary container to download packages
echo "   Creating temporary container to download .deb files..."
TMP_CONTAINER="vendor-apt-$$"

docker run --name "$TMP_CONTAINER" -d "$BASE_IMAGE" sleep 300 > /dev/null

# Update and download packages without installing
docker exec "$TMP_CONTAINER" bash -c "
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y --download-only \
        build-essential \
        cmake \
        git \
        libgtest-dev \
        libgmock-dev \
        gcovr \
        lcov \
        valgrind \
        clang-format \
        clang-tidy
"

# Copy downloaded packages
echo "   Copying .deb files from container..."
docker cp "$TMP_CONTAINER:/var/cache/apt/archives/." "$APT_CACHE_DIR/"

# Clean up container
docker stop "$TMP_CONTAINER" > /dev/null
docker rm "$TMP_CONTAINER" > /dev/null

# Count and clean up
cd "$APT_CACHE_DIR"
rm -f lock partial/*.tmp *.tmp 2>/dev/null || true
DEB_COUNT=$(ls -1 *.deb 2>/dev/null | wc -l)
echo "   ✓ Vendored $DEB_COUNT .deb packages ($(du -sh . | cut -f1))"

# ===== 3. Generate Manifest =====
echo ""
echo "[3/3] Generating manifest..."
cd "$VENDOR_DIR"

cat > MANIFEST.txt << EOF
Vendored Dependencies Manifest
Generated: $(date)
===============================

Ubuntu Base Image:
  Image: ubuntu:22.04
  File: base-image/ubuntu-22.04.tar
  Size: $(du -h base-image/ubuntu-22.04.tar | cut -f1)
  SHA256: $(sha256sum base-image/ubuntu-22.04.tar | cut -d' ' -f1)

APT Packages:
  Count: $DEB_COUNT packages
  Directory: apt-packages/
  Size: $(du -sh apt-packages/ | cut -f1)

Total Size: $(du -sh . | cut -f1)

Package List:
EOF

ls -1 apt-packages/*.deb | xargs -n1 basename >> MANIFEST.txt

echo "   ✓ Manifest created"

# ===== Summary =====
echo ""
echo "================================================"
echo "  Vendoring Complete!"
echo "================================================"
cat MANIFEST.txt
echo ""
echo "Next steps:"
echo "  1. Verify: ./verify-vendor.sh"
echo "  2. Build:  ./build.sh vendored"
echo "  3. Test:   ./test.sh"
echo "================================================"
