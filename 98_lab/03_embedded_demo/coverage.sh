#!/bin/bash
# Generate code coverage reports using gcov

set -e

# Determine which image to use
if docker images | grep -q "embedded-c-dev.*vendored"; then
    IMAGE="embedded-c-dev:vendored"
elif docker images | grep -q "embedded-c-dev.*latest"; then
    IMAGE="embedded-c-dev:latest"
else
    echo "ERROR: No embedded-c-dev image found!"
    echo "Please run ./build.sh first"
    exit 1
fi

echo "================================================"
echo "  Generating Code Coverage Report"
echo "================================================"
echo ""

# Clean previous coverage data
rm -rf project/build/artifacts/gcov 2>/dev/null || true

# Run coverage analysis
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project &&
        ceedling clobber &&
        ceedling gcov:all &&
        mkdir -p build/artifacts/gcov &&
        gcovr --html --html-details \
              --output build/artifacts/gcov/GcovCoverageResults.html \
              --root . \
              --exclude 'build/.*' \
              --exclude 'test/.*' \
              --gcov-ignore-parse-errors
    "

# Fix ownership
sudo chown -R $(id -u):$(id -g) project/build 2>/dev/null || true

# Check if report was generated
if [ -f "project/build/artifacts/gcov/GcovCoverageResults.html" ]; then
    echo ""
    echo "================================================"
    echo "  Coverage Report Generated"
    echo "================================================"
    echo ""
    echo "HTML report available at:"
    echo "  project/build/artifacts/gcov/GcovCoverageResults.html"
    echo ""
    echo "To view the report:"
    echo "  ./serve-coverage.sh"
    echo "  # Then open http://localhost:8081"
else
    echo ""
    echo "⚠ Warning: Coverage HTML report not found"
    echo "Checking what was generated..."
    ls -la project/build/artifacts/ 2>/dev/null || echo "No artifacts directory"
    ls -la project/build/gcov/ 2>/dev/null || echo "No gcov directory"
fi
