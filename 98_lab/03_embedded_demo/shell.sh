#!/bin/bash
# Open an interactive shell in the container

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

echo "Opening shell in $IMAGE..."
echo ""

docker run --rm -it \
    -v "$(pwd)/project:/project" \
    $IMAGE \
    /bin/bash
