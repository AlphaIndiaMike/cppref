#!/bin/bash
# =============================================================================
# serve.sh - Serve documentation locally for preview
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/project"
DOCS_DIR="${PROJECT_DIR}/docs/html"
PORT="${1:-8080}"

if [[ ! -d "${DOCS_DIR}" ]]; then
    echo "Error: Documentation not found at ${DOCS_DIR}"
    echo "Run ./docs.sh or ./doxygen.sh first."
    exit 1
fi

echo "Serving documentation at http://localhost:${PORT}"
echo "Press Ctrl+C to stop"
echo ""

# Try python3 first, then python
if command -v python3 &> /dev/null; then
    cd "${DOCS_DIR}" && python3 -m http.server "${PORT}"
elif command -v python &> /dev/null; then
    cd "${DOCS_DIR}" && python -m http.server "${PORT}"
else
    # Fall back to Docker
    docker run --rm \
        -v "${DOCS_DIR}:/docs" \
        -w /docs \
        -p "${PORT}:${PORT}" \
        python:3-alpine \
        python -m http.server "${PORT}"
fi
