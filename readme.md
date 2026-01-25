## Proposed Project Structure

```
my_project/
│
├── CMakeLists.txt              # Root CMake (project definition)
├── 97_cmake/                      # CMake modules and utilities
│   ├── CompilerWarnings.cmake
│   ├── StaticAnalysis.cmake
│   └── Doxygen.cmake
│
├── 00_components/                 # Your internal components/libraries
│   ├── CMakeLists.txt          # Adds all components
│   │
│   ├── core/                   # Example component
│   │   ├── CMakeLists.txt
│   │   ├── include/core/
│   │   │   ├── types.h
│   │   │   └── logger.h
│   │   └── src/
│   │       ├── logger.cc
│   │       └── internal_utils.h
│   │
│   ├── network/                # Another component
│   │   ├── CMakeLists.txt
│   │   ├── include/network/
│   │   │   ├── http_client.h
│   │   │   └── tcp_socket.h
│   │   └── src/
│   │       ├── http_client.cc
│   │       └── tcp_socket.cc
│   │
│   └── database/               # Another component
│       ├── CMakeLists.txt
│       ├── include/database/
│       └── src/
│
├── 01_app/                       # Executables / applications
│   ├── CMakeLists.txt
│   │
│   ├── main_app/               # Primary application
│   │   ├── CMakeLists.txt
│   │   └── main.cc
│   │
│   └── cli_tool/               # Secondary executable
│       ├── CMakeLists.txt
│       └── main.cc
│
├── 02_vendor/                     # Third-party dependencies (vendored)
│   ├── CMakeLists.txt
│   │
│   ├── json/                   # Example: nlohmann/json
│   │   └── ...
│   │
│   ├── spdlog/                 # Example: spdlog
│   │   └── ...
│   │
│   └── googletest/             # GTest/GMock
│       └── ...
│
├── 03_tests/                      # All tests
│   ├── CMakeLists.txt
│   │
│   ├── unit/                   # Unit tests (per component)
│   │   ├── CMakeLists.txt
│   │   ├── core/
│   │   │   └── logger_test.cc
│   │   ├── network/
│   │   │   └── http_client_test.cc
│   │   └── database/
│   │       └── ...
│   │
│   └── integration/            # Integration tests
│       ├── CMakeLists.txt
│       └── ...
│
├── 04_docs/                       # Documentation
│   ├── Doxyfile
│   ├── .clang-uml
│   ├── images/
│   └── uml/
│
├── 05_resources/                  # Runtime resources
│   ├── config/
│   │   ├── default.yaml
│   │   └── production.yaml
│   ├── schemas/
│   │   └── api_schema.json
│   └── assets/
│       └── ...
│
├── 99_project_utils/                    # Build/utility scripts
│   ├── build.sh
│   ├── run_tests.sh
│   └── generate_docs.sh
│
├── .clang-format               # Code formatting
├── .clang-tidy                 # Static analysis
├── .gitignore
└── README.md
```
