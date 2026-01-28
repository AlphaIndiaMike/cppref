cmake_minimum_required(VERSION 3.14)
project(presenter_cli VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# Project Settings
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Options
# ============================================================================

option(ENABLE_TESTING "Enable unit tests" ON)
option(ENABLE_COVERAGE "Enable code coverage" ON)

# ============================================================================
# Compiler Flags
# ============================================================================

# Warnings (Google Style recommends treating warnings seriously)
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

# Coverage flags
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# ============================================================================
# Source Files
# ============================================================================

# Main library (code)
add_library(${PROJECT_NAME}_lib

)

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Main executable (if applicable)
add_executable(${PROJECT_NAME}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
)

# ============================================================================
# Testing
# ============================================================================

if(ENABLE_TESTING)
    enable_testing()

    # Find Google Test
    find_package(GTest REQUIRED)
    include(GoogleTest)

    # Test executable
    add_executable(${PROJECT_NAME}_tests

        # Add more test files here
    )

    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE
            ${PROJECT_NAME}_lib
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main
    )

    # Auto-discover tests
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
