cmake_minimum_required(VERSION 3.14)

#[[-----------------------------------
Project Settings
-------------------------------------]]
project(PresenterProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#[[-----------------------------------
Component Registry
-------------------------------------]]
set(COMPONENTS_LIST "")

message("===========================================")
message("Building Components:")
message("===========================================")

#[[-----------------------------------
Add Components
-------------------------------------]]
add_subdirectory(01_main/presenters/cli/integration)
list(APPEND COMPONENTS_LIST "CLI Shell")

# add_subdirectory(01_main/other/component)
# list(APPEND COMPONENTS_LIST "Other Component")

message("===========================================")
message("Components added:")
foreach(comp ${COMPONENTS_LIST})
    message("  ✓ ${comp}")
endforeach()
message("===========================================")

#[[-----------------------------------
Main Executable
-------------------------------------]]
add_executable(MainApp
    01_main/entry/main.cc
)

target_link_libraries(MainApp PRIVATE
    cli_shell_lib
    # other_component_lib
)
