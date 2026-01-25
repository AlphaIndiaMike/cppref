# Example Embedded C Project

This is a minimal embedded C project demonstrating:
- GPIO driver with comprehensive API
- Circular buffer implementation
- Unit tests with Ceedling
- API documentation with Doxygen

## Modules

### GPIO Driver (`gpio_driver.h/c`)
Simple GPIO interface for embedded systems with:
- Pin initialization and configuration
- Digital read/write operations
- Toggle functionality
- Input pull-up support
- Full error checking

### Circular Buffer (`circular_buffer.h/c`)
Thread-safe byte buffer for communication interfaces:
- Fixed-size FIFO buffer
- Wraparound handling
- Size and availability tracking
- Full/empty detection

## Building the Demo

```bash
# Using Docker shell
cd /home/claude/embedded-c-docker
./shell.sh

# Inside container
cd /project
mkdir -p build && cd build
cmake ..
make
./embedded_demo
```

## Running Tests

```bash
# From host
./test.sh

# Or manually in container
./shell.sh
cd /project
ceedling test:all
ceedling test:gpio_driver
ceedling test:circular_buffer
```

## Generating Documentation

```bash
# From host
./docs.sh
./serve.sh  # View at http://localhost:8080

# Or manually in container
./shell.sh
cd /project
doxygen Doxyfile
```

## Code Coverage

```bash
# From host
./coverage.sh
./serve-coverage.sh  # View at http://localhost:8081

# Or manually
./shell.sh
cd /project
ceedling gcov:all
```

## Project Structure

```
project/
├── include/              # Public API headers
│   ├── gpio_driver.h
│   └── circular_buffer.h
├── src/                  # Implementation
│   ├── gpio_driver.c
│   ├── circular_buffer.c
│   └── main.c           # Demo application
├── test/                 # Unit tests
│   ├── test_gpio_driver.c
│   └── test_circular_buffer.c
├── CMakeLists.txt       # Build configuration
├── project.yml          # Ceedling configuration
└── Doxyfile             # Documentation configuration
```

## Adding New Modules

1. Create `include/my_module.h`
2. Create `src/my_module.c`
3. Create `test/test_my_module.c`
4. Run `ceedling test:my_module`

Ceedling automatically discovers and builds new modules!

## Documentation Style

All public APIs are documented with Doxygen:
```c
/**
 * @brief Brief description
 * 
 * Detailed description of what the function does.
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return 0 on success, -1 on error
 * 
 * @pre Preconditions
 * @post Postconditions
 * 
 * @code
 * // Usage example
 * result = my_function(arg1, arg2);
 * @endcode
 */
int my_function(int param1, int param2);
```

## Test Style

Tests use Unity framework:
```c
void test_my_feature(void) {
    // Setup
    int result;
    
    // Execute
    result = my_function(1, 2);
    
    // Verify
    TEST_ASSERT_EQUAL(3, result);
}
```
