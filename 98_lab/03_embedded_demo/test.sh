#!/bin/bash
# Run unit tests using Ceedling

set -e

# Determine which image to use
if docker images | grep -q "embedded-c-dev.*vendored"; then
    IMAGE="embedded-c-dev:vendored"
    echo "Using vendored image"
elif docker images | grep -q "embedded-c-dev.*latest"; then
    IMAGE="embedded-c-dev:latest"
    echo "Using latest image"
else
    echo "ERROR: No embedded-c-dev image found!"
    echo "Please run ./build.sh first"
    exit 1
fi

echo "================================================"
echo "  Running Unit Tests with Ceedling"
echo "================================================"
echo ""

# Run tests
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    run-tests

echo ""
echo "================================================"
echo "  Test Results Summary"
echo "================================================"
echo ""
echo "Test artifacts are in: project/build/test/"
echo ""
echo "To view detailed results:"
echo "  cat project/build/test/results/*.pass"
echo ""
echo "To run coverage analysis:"
echo "  ./coverage.sh"

# Fix ownership
sudo chown -R $(id -u):$(id -g) project/build 2>/dev/null || true
