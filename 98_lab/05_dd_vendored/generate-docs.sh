#!/bin/bash
set -e

PROJECT_DIR="${PROJECT_DIR:-/project}"
OUTPUT_DIR="${OUTPUT_DIR:-/project/docs}"
DOXYFILE="${DOXYFILE:-/project/Doxyfile}"
CLANG_UML_CONFIG="${CLANG_UML_CONFIG:-/project/.clang-uml}"

echo "================================================"
echo "  C++ Documentation Generator"
echo "================================================"
echo ""
echo "Project:    $PROJECT_DIR"
echo "Output:     $OUTPUT_DIR"
echo ""

# Create output directories
mkdir -p "$OUTPUT_DIR/uml"
mkdir -p "$OUTPUT_DIR/html"

# Step 1: Generate UML diagrams with clang-uml
if [ -f "$CLANG_UML_CONFIG" ]; then
    echo "[1/4] Generating UML diagrams with clang-uml..."
    cd "$PROJECT_DIR"
    
    # Generate compile_commands.json if not exists
    if [ ! -f "build/compile_commands.json" ]; then
        echo "      Creating compile_commands.json..."
        mkdir -p build
        cd build
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. 2>/dev/null || true
        cd ..
    fi
    
    clang-uml --config "$CLANG_UML_CONFIG" || echo "      Warning: clang-uml had issues (continuing)"
    echo "      UML diagrams generated."
else
    echo "[1/4] Skipping clang-uml (no .clang-uml config found)"
fi

# Step 2: Convert PlantUML to images
echo "[2/4] Rendering PlantUML diagrams..."
if ls "$OUTPUT_DIR/uml"/*.puml 1> /dev/null 2>&1; then
    plantuml -tsvg "$OUTPUT_DIR/uml/"*.puml || true
    plantuml -tpng "$OUTPUT_DIR/uml/"*.puml || true
    echo "      Diagrams rendered to SVG and PNG."
else
    echo "      No PlantUML files found."
fi

# Step 3: Run Doxygen
echo "[3/4] Generating Doxygen documentation..."
if [ -f "$DOXYFILE" ]; then
    cd "$PROJECT_DIR"
    doxygen "$DOXYFILE"
    echo "      Doxygen complete."
else
    echo "      ERROR: Doxyfile not found at $DOXYFILE"
    exit 1
fi

# Step 4: Summary
echo "[4/4] Documentation generation complete!"
echo ""
echo "Output locations:"
echo "  - HTML:     $OUTPUT_DIR/html/index.html"
echo "  - UML:      $OUTPUT_DIR/uml/"
if [ -f "$OUTPUT_DIR/xml/index.xml" ]; then
    echo "  - XML:      $OUTPUT_DIR/xml/ (for PLM import)"
fi
echo ""
echo "================================================"
