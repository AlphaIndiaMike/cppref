# Example C++ Project

This directory contains example C++ code to demonstrate the documentation generation.

## Code Structure

```
project/
├── include/
│   ├── Calculator.h      - Simple calculator class
│   └── StringUtils.h     - String utility functions
├── src/
│   ├── Calculator.cpp    - Calculator implementation
│   ├── StringUtils.cpp   - StringUtils implementation
│   └── main.cpp          - Demo application
└── CMakeLists.txt        - Build configuration
```

## Classes

### Calculator
A simple calculator class that performs basic arithmetic operations with method chaining.

Features:
- Add, subtract, multiply, divide
- Method chaining: `calc.add(10).multiply(2).subtract(5)`
- Exception handling for division by zero
- String representation of results

### StringUtils
A utility class with static methods for string manipulation.

Features:
- toUpper / toLower - Case conversion
- trim - Remove whitespace
- split - Split by delimiter
- startsWith / endsWith - Prefix/suffix checking
- reverse - Reverse a string

## Testing the Documentation

After running `./docs.sh`, the generated documentation will show:

1. **API Documentation** (Doxygen HTML)
   - Class hierarchies
   - Member function documentation
   - Call graphs
   - File dependencies

2. **UML Diagrams**
   - Class diagrams showing Calculator and StringUtils
   - Sequence diagrams showing method calls in main()
   - Include dependency graphs

## Building the Code

```bash
# Inside the Docker container
cd /project
mkdir -p build && cd build
cmake ..
make
./DocumentationDemo
```

Or using the shell script:
```bash
./shell.sh
# Then inside container:
cd build && cmake .. && make && ./DocumentationDemo
```
