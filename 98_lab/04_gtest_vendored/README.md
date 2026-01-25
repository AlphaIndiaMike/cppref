# Google Test Docker Environment

A Docker-based C++17 unit testing environment with Google Test and Google Mock.

## 🎯 Reproducible Builds (RECOMMENDED)

This project supports **vendored dependencies** for fully reproducible, offline builds.

### Quick Start (Vendored)

```bash
# 1. Download all dependencies (one-time setup)
./vendor.sh

# 2. Build with vendored dependencies (fully offline)
./build.sh vendored

# 3. Run tests
./test.sh

# 4. Run with coverage
./coverage.sh
```

### Why Vendor?

✅ **Future-proof** - Works identically in 5+ years  
✅ **Offline builds** - No internet required after initial vendor  
✅ **Reproducible** - Same input = same output, always  
✅ **CI/CD friendly** - Fast, reliable builds  

## Quick Start (Traditional)

```bash
# Build the Docker image
./build.sh

# Run all tests
./test.sh

# Run tests with coverage
./coverage.sh

# Interactive shell
./shell.sh
```

## Scripts

| Script | Description |
|--------|-------------|
| `vendor.sh` | **Download all dependencies for offline builds (run once)** |
| `build.sh` | Build Docker image (supports `vendored` mode) |
| `build-vendored.sh` | Build using only vendored dependencies |
| `verify-vendor.sh` | Verify vendored dependencies are complete |
| `test.sh` | Build and run all tests |
| `coverage.sh` | Run tests with coverage report |
| `shell.sh` | Interactive shell in container |

## Project Structure

```
.
├── Dockerfile
├── build.sh
├── test.sh
├── coverage.sh
├── shell.sh
└── project/
    ├── CMakeLists.txt
    ├── include/
    │   ├── calculator.h
    │   └── string_utils.h
    ├── src/
    │   ├── calculator.cc
    │   ├── string_utils.cc
    │   └── main.cc
    └── tests/
        ├── calculator_test.cc
        └── string_utils_test.cc
```

## Usage Without Docker

```bash
cd project
mkdir build && cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
```

## Manual Docker Commands

```bash
# Build image
docker build -t gtest-dev .

# Run tests
docker run --rm -v $(pwd)/project:/project -w /project gtest-dev \
    bash -c "mkdir -p build && cd build && cmake .. && make && ctest --output-on-failure"

# Run specific test
docker run --rm -v $(pwd)/project:/project -w /project gtest-dev \
    bash -c "cd build && ctest -R Calculator -V"

# Interactive
docker run --rm -it -v $(pwd)/project:/project -w /project gtest-dev bash
```

## Adding New Tests

1. Create a new test file in `project/tests/` (e.g., `my_feature_test.cc`)
2. Add the file to `CMakeLists.txt` under `add_executable(${PROJECT_NAME}_tests ...)`
3. Tests are auto-discovered by `gtest_discover_tests()`

## Test File Template

```cpp
#include "my_feature.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace my_project {
namespace {

TEST(MyFeatureTest, TestName) {
    EXPECT_EQ(expected, actual);
}

}  // namespace
}  // namespace my_project
```

## Tools Included

- GCC (C++17)
- CMake
- Google Test / Google Mock
- gcovr / lcov (coverage)
- Valgrind (memory)
- clang-format / clang-tidy
