#!/bin/bash
# Clean build artifacts

echo "================================================"
echo "  Cleaning Build Artifacts"
echo "================================================"
echo ""

# Clean Ceedling build directory
if [ -d "project/build" ]; then
    echo "Removing project/build/..."
    rm -rf project/build
fi

# Clean Doxygen output
if [ -d "project/docs" ]; then
    echo "Removing project/docs/..."
    rm -rf project/docs
fi

echo ""
echo "✓ Clean complete!"
echo ""
echo "Build artifacts removed. Source code unchanged."
