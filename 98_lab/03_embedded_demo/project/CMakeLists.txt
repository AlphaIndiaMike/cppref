cmake_minimum_required(VERSION 3.10)
project(EmbeddedCDemo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/gpio_driver.c
    src/circular_buffer.c
    src/main.c
)

# Create executable
add_executable(embedded_demo ${SOURCES})

# Optional: Add compile definitions
target_compile_definitions(embedded_demo PRIVATE
    # Add any defines here if needed
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
