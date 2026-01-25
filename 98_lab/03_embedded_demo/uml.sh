#!/bin/bash
# Generate UML diagrams using clang-uml

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
echo "  Generating UML Diagrams with clang-uml"
echo "================================================"
echo ""

# Generate compile_commands.json first (required by clang-uml)
echo "1. Generating compilation database..."
docker run --rm \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project &&
        mkdir -p build &&
        cd build &&
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. > /dev/null 2>&1
    "

echo "   ✓ Compilation database created"
echo ""

# Generate UML diagrams
echo "2. Generating UML diagrams..."
docker run --rm \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project &&
        mkdir -p docs/uml &&
        clang-uml --config .clang-uml
    "

echo "   ✓ UML diagrams generated (.puml files)"
echo ""

# Convert PlantUML to SVG
echo "3. Rendering PlantUML diagrams to SVG..."
docker run --rm \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project/docs/uml &&
        for file in *.puml; do
            if [ -f \"\$file\" ]; then
                plantuml -tsvg \"\$file\"
                echo \"   ✓ Rendered: \${file%.puml}.svg\"
            fi
        done
    "

echo ""
echo "================================================"
echo "  UML Diagrams Generated Successfully!"
echo "================================================"
echo ""
echo "Generated diagrams in project/docs/uml/:"
echo ""

# List generated diagrams
docker run --rm \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project/docs/uml 2>/dev/null &&
        ls -1 *.svg 2>/dev/null | while read file; do
            echo \"  • \$file\"
        done || echo '  (No diagrams generated yet)'
    "

