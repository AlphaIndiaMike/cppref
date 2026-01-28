#!/bin/bash
# Vendor Dependencies Script for Embedded C Project
# Downloads and stores all external dependencies for offline/reproducible builds

set -e

# Check if docker needs sudo
if ! docker ps >/dev/null 2>&1; then
    echo "Docker requires sudo - will use 'sudo docker' for all operations"
    DOCKER_CMD="sudo docker"
else
    DOCKER_CMD="docker"
fi

VENDOR_DIR="$(pwd)/vendored"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo "================================================"
echo "  Vendoring Dependencies for Embedded C Project"
echo "================================================"
echo "Target: $VENDOR_DIR"
echo ""

# Create vendor directory structure
mkdir -p "$VENDOR_DIR"/{base-image,plantuml,ruby-gems,apt-packages}

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

echo "$PLANTUML_VERSION" > "$VENDOR_DIR/plantuml/VERSION"

# ===== 2. clang-uml Source =====
echo ""
echo "[2/6] Vendoring clang-uml..."
CLANG_UML_VERSION="0.6.2"  # Pin to specific version
CLANG_UML_URL="https://github.com/bkryza/clang-uml/archive/refs/tags/${CLANG_UML_VERSION}.tar.gz"
CLANG_UML_FILE="$VENDOR_DIR/clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz"

mkdir -p "$VENDOR_DIR/clang-uml"

if [ -f "$CLANG_UML_FILE" ]; then
    echo "   ✓ Already vendored: clang-uml-${CLANG_UML_VERSION}.tar.gz"
else
    echo "   Downloading clang-uml ${CLANG_UML_VERSION}..."
    wget -q --show-progress "$CLANG_UML_URL" -O "$CLANG_UML_FILE"
    echo "   ✓ Vendored: clang-uml-${CLANG_UML_VERSION}.tar.gz"
fi

echo "$CLANG_UML_VERSION" > "$VENDOR_DIR/clang-uml/VERSION"

# ===== 3. Ruby Gems (Ceedling and dependencies) =====
echo ""
echo "[3/6] Vendoring Ruby Gems (Ceedling)..."
GEMS_DIR="$VENDOR_DIR/ruby-gems"

if [ -d "$GEMS_DIR/cache" ] && [ "$(ls -A $GEMS_DIR/cache/*.gem 2>/dev/null | wc -l)" -gt 0 ]; then
    echo "   ✓ Already vendored: $(ls -1 $GEMS_DIR/cache/*.gem 2>/dev/null | wc -l) gems"
else
    echo "   Creating temporary container to download gems..."
    TMP_CONTAINER="vendor-gems-$$"

    $DOCKER_CMD run --name "$TMP_CONTAINER" -d ubuntu:22.04 sleep 300 > /dev/null

    $DOCKER_CMD exec "$TMP_CONTAINER" bash -c "
        apt-get update && apt-get install -y ruby-full wget
        gem install --no-document ceedling -v 1.0.1
    "

    # Copy gem cache
    echo "   Copying gem files from container..."
    $DOCKER_CMD cp "$TMP_CONTAINER:/var/lib/gems/." "$GEMS_DIR/"

    # Cleanup container
    $DOCKER_CMD stop "$TMP_CONTAINER" > /dev/null
    $DOCKER_CMD rm "$TMP_CONTAINER" > /dev/null

    GEM_COUNT=$(ls -1 "$GEMS_DIR/cache"/*.gem 2>/dev/null | wc -l)
    echo "   ✓ Vendored $GEM_COUNT gems ($(du -sh $GEMS_DIR | cut -f1))"
fi

# ===== 4. Ubuntu Base Image =====
echo ""
echo "[4/6] Vendoring Ubuntu base image..."
BASE_IMAGE="ubuntu:22.04"
BASE_IMAGE_FILE="$VENDOR_DIR/base-image/ubuntu-22.04.tar"

if [ -f "$BASE_IMAGE_FILE" ]; then
    echo "   ✓ Already vendored: ubuntu-22.04.tar"
else
    echo "   Pulling and saving Ubuntu 22.04 image..."
    $DOCKER_CMD pull "$BASE_IMAGE"
    $DOCKER_CMD save "$BASE_IMAGE" -o "$BASE_IMAGE_FILE"
    echo "   ✓ Vendored: ubuntu-22.04.tar ($(du -h "$BASE_IMAGE_FILE" | cut -f1))"
fi

# ===== 5. APT Packages =====
echo ""
echo "[5/6] Vendoring APT packages..."
APT_CACHE_DIR="$VENDOR_DIR/apt-packages"

if [ -d "$APT_CACHE_DIR" ] && [ "$(ls -1 $APT_CACHE_DIR/*.deb 2>/dev/null | wc -l)" -gt 0 ]; then
    echo "   ✓ Already vendored: $(ls -1 $APT_CACHE_DIR/*.deb 2>/dev/null | wc -l) packages"
else
    echo "   Creating temporary container to download .deb files..."
    TMP_CONTAINER="vendor-apt-$$"

    $DOCKER_CMD run --name "$TMP_CONTAINER" -d "$BASE_IMAGE" sleep 300 > /dev/null

    # Download packages
    $DOCKER_CMD exec "$TMP_CONTAINER" bash -c "
        export DEBIAN_FRONTEND=noninteractive
        apt-get update
        apt-get install -y --download-only \
            build-essential \
            gcc \
            gdb \
            make \
            git \
            wget \
            curl \
            gcovr \
            lcov \
            valgrind \
            ruby-full \
            doxygen \
            graphviz \
            default-jre \
            cmake \
            clang-15 \
            libclang-15-dev \
            libclang-cpp15-dev \
            llvm-15-dev \
            libyaml-cpp-dev
    "

    # Copy downloaded packages
    echo "   Copying .deb files from container..."
    mkdir -p "$APT_CACHE_DIR"
    $DOCKER_CMD cp "$TMP_CONTAINER:/var/cache/apt/archives/." "$APT_CACHE_DIR/"

    # Cleanup
    $DOCKER_CMD stop "$TMP_CONTAINER" > /dev/null
    $DOCKER_CMD rm "$TMP_CONTAINER" > /dev/null

    # Remove lock files
    cd "$APT_CACHE_DIR"
    rm -f lock partial/*.tmp *.tmp 2>/dev/null || true
    DEB_COUNT=$(ls -1 *.deb 2>/dev/null | wc -l)
    echo "   ✓ Vendored $DEB_COUNT .deb packages ($(du -sh . | cut -f1))"
fi

# ===== 6. Generate Manifest =====
echo ""
echo "[6/6] Generating manifest..."
cd "$VENDOR_DIR"

GEM_COUNT=$(ls -1 "$GEMS_DIR/cache"/*.gem 2>/dev/null | wc -l || echo "0")
DEB_COUNT=$(ls -1 "$APT_CACHE_DIR"/*.deb 2>/dev/null | wc -l || echo "0")

cat > MANIFEST.txt << EOF
Vendored Dependencies Manifest - Embedded C Project
Generated: $(date)
========================================================

PlantUML:
  Version: $PLANTUML_VERSION
  File: plantuml/plantuml-${PLANTUML_VERSION}.jar
  SHA256: $(sha256sum plantuml/plantuml-${PLANTUML_VERSION}.jar | cut -d' ' -f1)

clang-uml:
  Version: $CLANG_UML_VERSION
  File: clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz
  SHA256: $(sha256sum clang-uml/clang-uml-${CLANG_UML_VERSION}.tar.gz | cut -d' ' -f1)

Ruby Gems (Ceedling):
  Gems: $GEM_COUNT gems
  Directory: ruby-gems/
  Size: $(du -sh ruby-gems/ | cut -f1)
  Key gems: ceedling, unity, cmock

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
========================================================
EOF

echo "   ✓ Manifest created"

# ===== Fix Ownership =====
echo ""
echo "Fixing file ownership..."
if [ -n "$SUDO_USER" ]; then
    # Script was run with sudo, chown to the actual user
    chown -R $SUDO_USER:$SUDO_USER "$VENDOR_DIR"
    echo "   ✓ Changed ownership to $SUDO_USER"
elif [ "$(id -u)" != "0" ]; then
    # Script run as normal user, but files created by docker are root-owned
    # Need to fix with sudo
    if command -v sudo >/dev/null 2>&1; then
        sudo chown -R $(id -u):$(id -g) "$VENDOR_DIR"
        echo "   ✓ Changed ownership to $(whoami)"
    else
        echo "   ⚠ Warning: Files owned by root, may need: sudo chown -R \$(id -u):\$(id -g) $VENDOR_DIR"
    fi
fi

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
echo "  4. Docs:   ./docs.sh"
echo ""
echo "Storage recommendations:"
echo "  - Total size: $(sudo du -sh $VENDOR_DIR | cut -f1)"
echo "  - Commit to git with LFS if < 500MB"
echo "  - Store in artifact repository for larger sizes"
echo "================================================"
