#!/bin/bash
# Verify vendored dependencies are complete and valid

set -e

VENDOR_DIR="vendored"
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

# Check PlantUML
echo "[1/5] Checking PlantUML..."
if ls plantuml/plantuml-*.jar >/dev/null 2>&1; then
    JAR_SIZE=$(du -h plantuml/plantuml-*.jar | cut -f1)
    check_ok "PlantUML JAR found ($JAR_SIZE)"
    
    # Verify it's a valid JAR
    if file plantuml/plantuml-*.jar | grep -q "Java archive"; then
        check_ok "Valid JAR file format"
    else
        check_error "Invalid JAR file format"
    fi
else
    check_error "PlantUML JAR not found"
fi

# Check clang-uml
echo ""
echo "[2/5] Checking clang-uml..."
if ls clang-uml/clang-uml-*.tar.gz >/dev/null 2>&1; then
    TAR_SIZE=$(du -h clang-uml/clang-uml-*.tar.gz | cut -f1)
    check_ok "clang-uml tarball found ($TAR_SIZE)"
    
    # Verify it's a valid gzip
    if gzip -t clang-uml/clang-uml-*.tar.gz 2>/dev/null; then
        check_ok "Valid tar.gz format"
    else
        check_error "Invalid tar.gz format"
    fi
else
    check_error "clang-uml tarball not found"
fi

# Check Ubuntu base image
echo ""
echo "[3/5] Checking Ubuntu base image..."
if [ -f "base-image/ubuntu-22.04.tar" ]; then
    IMG_SIZE=$(du -h base-image/ubuntu-22.04.tar | cut -f1)
    check_ok "Ubuntu image found ($IMG_SIZE)"
    
    # Verify it's a valid tar
    if tar -tzf base-image/ubuntu-22.04.tar >/dev/null 2>&1; then
        check_ok "Valid Docker image tar"
    else
        check_error "Invalid Docker image format"
    fi
else
    check_error "Ubuntu base image not found"
fi

# Check APT packages
echo ""
echo "[4/5] Checking APT packages..."
if [ -d "apt-packages" ]; then
    DEB_COUNT=$(find apt-packages -name "*.deb" | wc -l)
    APT_SIZE=$(du -sh apt-packages | cut -f1)
    
    if [ "$DEB_COUNT" -gt 0 ]; then
        check_ok "Found $DEB_COUNT .deb packages ($APT_SIZE)"
        
        # Check for critical packages
        CRITICAL=(
            "doxygen"
            "graphviz"
            "clang-15"
            "libclang-15-dev"
            "libclang-cpp15-dev"
            "cmake"
            "build-essential"
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
echo "[5/5] Checking MANIFEST..."
if [ -f "MANIFEST.txt" ]; then
    check_ok "MANIFEST.txt exists"
    
    # Verify SHA256 checksums if sha256sum is available
    if command -v sha256sum >/dev/null 2>&1; then
        echo "   Verifying checksums..."
        
        verify_checksum() {
            local file=$1
            local manifest_sha=$(grep -A2 "$file" MANIFEST.txt | grep "SHA256:" | awk '{print $2}')
            
            if [ -n "$manifest_sha" ] && [ -f "$file" ]; then
                local actual_sha=$(sha256sum "$file" | cut -d' ' -f1)
                if [ "$manifest_sha" = "$actual_sha" ]; then
                    check_ok "Checksum verified: $(basename $file)"
                else
                    check_error "Checksum mismatch: $(basename $file)"
                fi
            fi
        }
        
        verify_checksum "plantuml/plantuml-*.jar" || true
        verify_checksum "clang-uml/clang-uml-*.tar.gz" || true
        verify_checksum "base-image/ubuntu-22.04.tar" || true
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
