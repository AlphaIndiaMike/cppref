#!/bin/bash
# Verify vendored dependencies are complete

set -e

VENDOR_DIR="./vendored"

echo "================================================"
echo "  Verifying Vendored Dependencies"
echo "================================================"
echo ""

# Function to check directory
check_dir() {
    local dir=$1
    local name=$2
    if [ -d "$dir" ]; then
        echo "✓ $name: $(du -sh $dir | cut -f1)"
        return 0
    else
        echo "✗ $name: NOT FOUND"
        return 1
    fi
}

# Function to check file
check_file() {
    local file=$1
    local name=$2
    if [ -f "$file" ]; then
        echo "✓ $name: $(du -h $file | cut -f1)"
        return 0
    else
        echo "✗ $name: NOT FOUND"
        return 1
    fi
}

ERRORS=0

# Check main directory
if [ ! -d "$VENDOR_DIR" ]; then
    echo "ERROR: vendored/ directory not found!"
    echo "Please run ./vendor.sh first"
    exit 1
fi

echo "Checking vendored dependencies..."
echo ""

# Check each component
check_dir "$VENDOR_DIR/apt-packages" "APT packages" || ERRORS=$((ERRORS+1))
check_dir "$VENDOR_DIR/ruby-gems" "Ruby gems" || ERRORS=$((ERRORS+1))
check_dir "$VENDOR_DIR/plantuml" "PlantUML" || ERRORS=$((ERRORS+1))
check_dir "$VENDOR_DIR/clang-uml" "clang-uml" || ERRORS=$((ERRORS+1))
check_file "$VENDOR_DIR/base-image/ubuntu-22.04.tar" "Ubuntu base image" || ERRORS=$((ERRORS+1))
check_file "$VENDOR_DIR/MANIFEST.txt" "Manifest" || ERRORS=$((ERRORS+1))

echo ""
echo "Checking file counts..."
DEB_COUNT=$(ls -1 $VENDOR_DIR/apt-packages/*.deb 2>/dev/null | wc -l || echo "0")
GEM_COUNT=$(ls -1 $VENDOR_DIR/ruby-gems/*/cache/*.gem 2>/dev/null | wc -l || echo "0")

echo "  .deb packages: $DEB_COUNT"
echo "  .gem files: $GEM_COUNT"

if [ "$DEB_COUNT" -lt 50 ]; then
    echo "  ⚠ Warning: Expected at least 50 .deb packages"
    ERRORS=$((ERRORS+1))
fi

if [ "$GEM_COUNT" -lt 4 ]; then
    echo "  ⚠ Warning: Expected at least 5 .gem files"
    ERRORS=$((ERRORS+1))
fi

echo ""
echo "Total size: $(du -sh $VENDOR_DIR | cut -f1)"
echo ""

if [ $ERRORS -eq 0 ]; then
    echo "================================================"
    echo "  ✓ All dependencies verified!"
    echo "================================================"
    echo ""
    echo "You can now build with:"
    echo "  ./build.sh vendored"
    echo ""
    exit 0
else
    echo "================================================"
    echo "  ✗ Verification failed with $ERRORS errors"
    echo "================================================"
    echo ""
    echo "Please run ./vendor.sh to download missing dependencies"
    echo ""
    exit 1
fi
