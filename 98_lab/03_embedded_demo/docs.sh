#!/bin/bash
# Generate documentation using Doxygen

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
echo "  Generating Documentation"
echo "================================================"
echo ""

echo "1. Generating UML diagrams with clang-uml..."
echo ""

# Generate compile_commands.json first (required by clang-uml)
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project &&
        mkdir -p build &&
        cd build &&
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. > /dev/null 2>&1
    "

# Generate UML diagrams
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project &&
        mkdir -p docs/uml &&
        clang-uml --config .clang-uml 2>&1 | grep -v 'Generating diagram' || true
    "

# Convert PlantUML to SVG
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    bash -c "
        cd /project/docs/uml &&
        for file in *.puml; do
            if [ -f \"\$file\" ]; then
                echo \"Converting \$file...\"
                plantuml -tsvg \"\$file\"
            fi
        done
    " 2>&1 | grep -v "Error reading"

echo "   ✓ PlantUML diagrams rendered"
echo ""

echo "2. Generating API documentation with Doxygen..."
echo ""

# Generate documentation
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    generate-docs

echo ""
echo "================================================"
echo "  Documentation Generated"
echo "================================================"
echo ""
echo "Generated documentation:"
echo "  • API docs: project/docs/html/index.html"
echo "  • UML diagrams: project/docs/uml/*.svg"
echo ""
echo "Available UML diagrams:"
echo "  • sequence_main.svg - Main execution flow"
echo "  • sequence_gpio.svg - GPIO operations"
echo "  • sequence_buffer.svg - Buffer operations"
echo "  • include_dependencies.svg - Include graph"
echo "  • struct_overview.svg - Data structures"
echo ""
