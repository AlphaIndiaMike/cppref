#!/bin/bash
# Vendor Dependencies Script
# Downloads and stores all external dependencies for offline/reproducible builds

set -e

VENDOR_DIR="$(pwd)/vendored"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo "================================================"
echo "  Vendoring Dependencies"
echo "================================================"
echo "Target: $VENDOR_DIR"
echo ""

# Create vendor directory structure
mkdir -p "$VENDOR_DIR"/{base-image,plantuml,clang-uml,apt-packages}

# ===== 1. PlantUML JAR =====
echo "[1/5] Vendoring PlantUML..."
PLANTUML_VERSION="1.2026.1"
PLANTUML_URL="https://github.com/plantuml/plantuml/releases/download/v${PLANTUML_VERSION}/plantuml-${PLANTUML_VERSION}.jar"
PLANTUML_FILE="$VENDOR_DIR/plantuml/plantuml-${PLANTUML_VERSION}.jar"

if [ -f "$PLANTUML_FILE" ]; then
    echo "   ✓ Already vendored: plantuml-${PLANTUML_VERSION}.jar"
else
    echo "   Downloading PlantUML ${PLANTUML_VERSION}..."
    wget -q --show-progress "$PLANTUML_URL" -O "$PLANTUML_FILE"
    echo "   ✓ Vendored: plantuml-${PLANTUML_VERSION}.jar"
fi

# Create version file
echo "$PLANTUML_VERSION" > "$VENDOR_DIR/plantuml/VERSION"

# ===== 2. clang-uml Source =====
echo ""
echo "[2/5] Vendoring clang-uml..."
CLANG_UML_VERSION="0.6.2"  # Pin to specific version
CLANG_UML_URL="https://github.com/bkryza/clang-uml/archive/refs/tags/${CLANG_UML_VERSION}.tar.gz"
CLANG_UML_FILE="$VENDOR_DIR/clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz"

if [ -f "$CLANG_UML_FILE" ]; then
    echo "   ✓ Already vendored: clang-uml-${CLANG_UML_VERSION}.tar.gz"
else
    echo "   Downloading clang-uml ${CLANG_UML_VERSION}..."
    wget -q --show-progress "$CLANG_UML_URL" -O "$CLANG_UML_FILE"
    echo "   ✓ Vendored: clang-uml-${CLANG_UML_VERSION}.tar.gz"
fi

# Create version file
echo "$CLANG_UML_VERSION" > "$VENDOR_DIR/clang-uml/VERSION"

# ===== 3. Ubuntu Base Image =====
echo ""
echo "[3/5] Vendoring Ubuntu base image..."
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

# ===== 4. APT Packages =====
echo ""
echo "[4/5] Vendoring APT packages..."
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
        wget \
        curl \
        python3 \
        python3-pip \
        default-jre \
        doxygen \
        graphviz \
        clang-15 \
        libclang-15-dev \
        libclang-cpp15-dev \
        llvm-15-dev \
        libyaml-cpp-dev
"

# Copy downloaded packages
echo "   Copying .deb files from container..."
docker cp "$TMP_CONTAINER:/var/cache/apt/archives/." "$APT_CACHE_DIR/"

# Clean up
docker stop "$TMP_CONTAINER" > /dev/null
docker rm "$TMP_CONTAINER" > /dev/null

# Count and clean up
cd "$APT_CACHE_DIR"
rm -f lock partial/*.tmp *.tmp 2>/dev/null || true
DEB_COUNT=$(ls -1 *.deb 2>/dev/null | wc -l)
echo "   ✓ Vendored $DEB_COUNT .deb packages ($(du -sh . | cut -f1))"

# ===== 5. Generate Manifest =====
echo ""
echo "[5/5] Generating manifest..."
cd "$VENDOR_DIR"

cat > MANIFEST.txt << EOF
Vendored Dependencies Manifest
Generated: $(date)
===============================

PlantUML:
  Version: $PLANTUML_VERSION
  File: plantuml/plantuml-${PLANTUML_VERSION}.jar
  SHA256: $(sha256sum plantuml/plantuml-${PLANTUML_VERSION}.jar | cut -d' ' -f1)

clang-uml:
  Version: $CLANG_UML_VERSION
  File: clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz
  SHA256: $(sha256sum clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz | cut -d' ' -f1)

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
EOF

echo "   ✓ Manifest created"

# ===== Summary =====
echo ""
echo "================================================"
echo "  Vendoring Complete!"
echo "================================================"
cat MANIFEST.txt
echo ""
echo "Next steps:"
echo "  1. Review: cat vendored/MANIFEST.txt"
echo "  2. Build:  ./build-vendored.sh"
echo "  3. Commit: git add vendored/ (or store separately)"
echo ""
echo "Storage recommendations:"
echo "  - Commit to git with LFS for <500MB total"
echo "  - Store in separate artifact repository for larger"
echo "  - Archive and backup for long-term preservation"
echo "================================================"
