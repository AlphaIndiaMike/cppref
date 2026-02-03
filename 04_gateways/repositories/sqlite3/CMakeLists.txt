cmake_minimum_required(VERSION 3.14)
project(sqlite3_repo VERSION 1.0.0 LANGUAGES C CXX)

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
    # -Wall
    # -Wextra
    # -Wpedantic
    # -Werror
)

# Coverage flags
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# ============================================================================
# SQLite3 Library (from source)
# ============================================================================

add_library(sqlite3
    ${CMAKE_CURRENT_SOURCE_DIR}/integration/sqlite3/sqlite3.c
)

target_include_directories(sqlite3
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/integration/sqlite3
)

# SQLite3 compile definitions for common features
target_compile_definitions(sqlite3
    PRIVATE
        SQLITE_ENABLE_FTS5
        SQLITE_ENABLE_JSON1
        SQLITE_ENABLE_RTREE
        SQLITE_THREADSAFE=1
)

# SQLite3 requires pthread and dl on Unix systems
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(sqlite3
        PUBLIC
            Threads::Threads
            ${CMAKE_DL_LIBS}
    )
endif()

# Suppress warnings for SQLite3 (third-party code)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sqlite3 PRIVATE -w)
endif()

# ============================================================================
# Source Files
# ============================================================================

# Main library (code)
add_library(${PROJECT_NAME}_lib
    src/account_repository.cc
    src/keyvalue_repository.cc
    src/timeseries_repository.cc
    integration/sqlite3_database_connector.cc
)

target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        sqlite3
)

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/integration/sqlite3
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Main executable (if applicable)
add_executable(${PROJECT_NAME}
    src/main.cc
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
        test/account_repository_test.cc
        test/keyvalue_repository_test.cc
        test/timeseries_repository_test.cc
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
