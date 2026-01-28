#!/bin/bash
# Verify vendored dependencies are complete and valid

set -e

VENDOR_DIR="90_vendored"
ERRORS=0
WARNINGS=0

echo "================================================"
echo "  Vendored Dependencies Verification"
echo "================================================"
echo ""

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

check_ok() {
    echo -e "${GREEN}✓${NC} $1"
}

check_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    WARNINGS=$((WARNINGS + 1))
}

check_error() {
    echo -e "${RED}✗${NC} $1"
    ERRORS=$((ERRORS + 1))
}

# Check vendor directory exists
if [ ! -d "$VENDOR_DIR" ]; then
    check_error "vendored/ directory not found"
    echo ""
    echo "Run ./vendor.sh to create it."
    exit 1
fi

cd "$VENDOR_DIR"

# Check Ubuntu base image
echo "[1/3] Checking Ubuntu base image..."
if [ -f "base-image/ubuntu-22.04.tar" ]; then
    IMG_SIZE=$(du -h base-image/ubuntu-22.04.tar | cut -f1)
    check_ok "Ubuntu image found ($IMG_SIZE)"

    # Verify it's a valid tar AND contains a Docker manifest
    if tar -tf base-image/ubuntu-22.04.tar manifest.json >/dev/null 2>&1; then
        check_ok "Valid Docker image tar"
    else
        check_error "Invalid Docker image: missing manifest.json"
    fi
else
    check_error "Ubuntu base image not found"
fi

# Check APT packages
echo ""
echo "[2/3] Checking APT packages..."
if [ -d "apt-packages" ]; then
    DEB_COUNT=$(find apt-packages -name "*.deb" | wc -l)
    APT_SIZE=$(du -sh apt-packages | cut -f1)

    if [ "$DEB_COUNT" -gt 0 ]; then
        check_ok "Found $DEB_COUNT .deb packages ($APT_SIZE)"

        # Check for critical packages
        CRITICAL=(
            "build-essential"
            "cmake"
            "libgtest-dev"
            "libgmock-dev"
            "gcovr"
            "lcov"
        )

        for pkg in "${CRITICAL[@]}"; do
            if find apt-packages -name "${pkg}_*.deb" | grep -q .; then
                check_ok "Critical package: $pkg"
            else
                check_error "Missing critical package: $pkg"
            fi
        done
    else
        check_error "No .deb files found in apt-packages/"
    fi
else
    check_error "apt-packages/ directory not found"
fi

# Check MANIFEST
echo ""
echo "[3/3] Checking MANIFEST..."
if [ -f "MANIFEST.txt" ]; then
    check_ok "MANIFEST.txt exists"

    # Verify SHA256 checksum if sha256sum is available
    if command -v sha256sum >/dev/null 2>&1; then
        echo "   Verifying checksums..."

        if [ -f "base-image/ubuntu-22.04.tar" ]; then
            manifest_sha=$(grep -A1 "Ubuntu Base Image:" MANIFEST.txt | grep "SHA256:" | awk '{print $2}')

            if [ -n "$manifest_sha" ]; then
                actual_sha=$(sha256sum "base-image/ubuntu-22.04.tar" | cut -d' ' -f1)
                if [ "$manifest_sha" = "$actual_sha" ]; then
                    check_ok "Checksum verified: ubuntu-22.04.tar"
                else
                    check_error "Checksum mismatch: ubuntu-22.04.tar"
                fi
            fi
        fi
    else
        check_warn "sha256sum not available, skipping checksum verification"
    fi
else
    check_error "MANIFEST.txt not found"
fi

# Summary
echo ""
echo "================================================"
echo "  Verification Summary"
echo "================================================"

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ All checks passed!${NC}"
    echo ""
    echo "Vendored dependencies are complete and valid."
    echo "You can build offline with: ./build.sh vendored"
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠ Verification completed with $WARNINGS warnings${NC}"
    echo ""
    echo "Dependencies should work but may have minor issues."
    exit 0
else
    echo -e "${RED}✗ Verification failed with $ERRORS errors and $WARNINGS warnings${NC}"
    echo ""
    echo "Please fix the issues above or re-run: ./vendor.sh"
    exit 1
fi
