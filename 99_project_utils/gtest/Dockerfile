# Google Test / Google Mock Development Environment
# C++17 with CMake, GTest, GMock

FROM ubuntu:22.04

LABEL maintainer="your-team"
LABEL description="C++17 unit testing environment with Google Test and Google Mock"

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libgtest-dev \
    libgmock-dev \
    gcovr \
    lcov \
    valgrind \
    clang-format \
    clang-tidy \
    && rm -rf /var/lib/apt/lists/*

# Build and install Google Test (some systems need this step)
WORKDIR /usr/src/gtest
RUN cmake . && make && cp lib/*.a /usr/lib/ 2>/dev/null || true

# Set up working directory
WORKDIR /project

# Default command: run all tests
CMD ["bash", "-c", "mkdir -p build && cd build && cmake .. && make && ctest --output-on-failure"]
