cmake_minimum_required(VERSION 3.14)
project(DocumentationDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Library with Calculator and StringUtils
add_library(${PROJECT_NAME}_lib
    src/Calculator.cpp
    src/StringUtils.cpp
)

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
)
