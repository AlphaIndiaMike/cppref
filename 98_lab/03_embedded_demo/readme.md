# Quick Test Guide - Embedded C Development Environment

## What's Included

**Example Embedded C Code:**
- `project/include/gpio_driver.h` - GPIO driver with full docs
- `project/src/gpio_driver.c` - Implementation (~130 lines)
- `project/include/circular_buffer.h` - Circular buffer with full docs
- `project/src/circular_buffer.c` - Implementation (~110 lines)
- `project/test/test_gpio_driver.c` - 13 unit tests
- `project/test/test_circular_buffer.c` - 13 unit tests

**Tools Included:**
- ✅ Ceedling 0.31.1 (unit testing framework)
- ✅ Unity (test assertions)
- ✅ CMock (mocking framework)
- ✅ gcov/gcovr (code coverage)
- ✅ Doxygen (API documentation)
- ✅ clang-uml (UML sequence diagrams, structure diagrams)
- ✅ PlantUML (diagram rendering)
- ✅ Valgrind (memory checking)

## Recommended: Vendored Build

```bash
# 1. Download all dependencies (~500MB-1GB, one-time)
./vendor.sh

# 2. Verify everything is complete
./verify-vendor.sh

# 3. Build using vendored dependencies (offline)
./build.sh vendored

# 4. Run unit tests
./test.sh

# 5. Generate code coverage
./coverage.sh

# 6. Generate documentation
./docs.sh
