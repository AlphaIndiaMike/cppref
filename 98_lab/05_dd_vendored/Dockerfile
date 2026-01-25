# Documentation Generation Environment
# Doxygen + clang-uml + PlantUML + Graphviz

FROM ubuntu:22.04

LABEL maintainer="your-team"
LABEL description="C++ documentation generator with UML diagrams"

ENV DEBIAN_FRONTEND=noninteractive

# Install base tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    default-jre \
    && rm -rf /var/lib/apt/lists/*

# Install Doxygen (latest stable)
RUN apt-get update && apt-get install -y \
    doxygen \
    && rm -rf /var/lib/apt/lists/*

# Install Graphviz (for Doxygen graphs)
RUN apt-get update && apt-get install -y \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Install PlantUML
RUN mkdir -p /opt/plantuml && \
    wget -q https://github.com/plantuml/plantuml/releases/download/v1.2024.0/plantuml-1.2024.0.jar \
    -O /opt/plantuml/plantuml.jar

ENV PLANTUML_JAR=/opt/plantuml/plantuml.jar

# Install clang-uml dependencies
RUN apt-get update && apt-get install -y \
    clang-15 \
    libclang-15-dev \
    libclang-cpp15-dev \
    llvm-15-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Build clang-uml from source
RUN git clone https://github.com/bkryza/clang-uml.git /tmp/clang-uml && \
    cd /tmp/clang-uml && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTS=OFF && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/clang-uml

# Create wrapper script for PlantUML
RUN echo '#!/bin/bash\njava -jar /opt/plantuml/plantuml.jar "$@"' > /usr/local/bin/plantuml && \
    chmod +x /usr/local/bin/plantuml

# Create documentation generation script
COPY generate-docs.sh /usr/local/bin/generate-docs
RUN chmod +x /usr/local/bin/generate-docs

WORKDIR /project

# Default command
CMD ["generate-docs"]
