# Embedded C Development Environment (ONLINE BUILD)
# Includes: Ceedling (unit testing), Doxygen (documentation), Coverage tools
# Downloads dependencies from internet

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV GEM_HOME=/usr/local/bundle
ENV BUNDLE_PATH=$GEM_HOME
ENV PATH=$GEM_HOME/bin:$PATH

# Install all required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    make \
    cmake \
    git \
    wget \
    curl \
    gcovr \
    lcov \
    valgrind \
    ruby-full \
    doxygen \
    graphviz \
    default-jre \
    clang-15 \
    libclang-15-dev \
    libclang-cpp15-dev \
    llvm-15-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Install PlantUML
RUN wget -q https://github.com/plantuml/plantuml/releases/download/v1.2024.0/plantuml-1.2024.0.jar \
    -O /opt/plantuml.jar && \
    echo '#!/bin/bash\njava -jar /opt/plantuml.jar "$@"' > /usr/local/bin/plantuml && \
    chmod +x /usr/local/bin/plantuml

# Build and install clang-uml
RUN git clone --depth 1 --branch 0.5.4 https://github.com/bkryza/clang-uml.git /tmp/clang-uml && \
    cd /tmp/clang-uml && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTS=OFF && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/clang-uml

# Install Ceedling
RUN gem install ceedling -v 0.31.1 --no-document

# Verify installations
RUN echo "=== Verifying installations ===" && \
    gcc --version && \
    ruby --version && \
    ceedling version && \
    doxygen --version && \
    gcovr --version && \
    echo "=== All tools installed successfully ==="

# Create helper scripts
RUN echo '#!/bin/bash\ncd /project && ceedling test:all' > /usr/local/bin/run-tests && \
    chmod +x /usr/local/bin/run-tests

RUN echo '#!/bin/bash\ncd /project && doxygen Doxyfile' > /usr/local/bin/generate-docs && \
    chmod +x /usr/local/bin/generate-docs

RUN echo '#!/bin/bash\ncd /project && ceedling gcov:all' > /usr/local/bin/run-coverage && \
    chmod +x /usr/local/bin/run-coverage

WORKDIR /project

CMD ["run-tests"]
