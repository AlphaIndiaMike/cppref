#!/bin/bash
# Clean build artifacts

echo "================================================"
echo "  Cleaning Build Artifacts"
echo "================================================"
echo ""

# Clean Ceedling build directory
if [ -d "build" ]; then
    echo "Removing build/..."
    sudo rm -rf build
fi

echo ""
echo "✓ Clean complete!"
echo ""
echo "Build artifacts removed. Source code unchanged."
