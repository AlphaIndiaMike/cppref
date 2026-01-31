cmake_minimum_required(VERSION 3.14)

#[[-----------------------------------
Project Settings
-------------------------------------]]
project(PresenterProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CUSTOM
set(PROJECT_SHOW_CMAKE_DEBUG_INFO OFF)


#[[-----------------------------------
Component Registry
-------------------------------------]]
message("===========================================")
message("Building Components:")
message("===========================================")

#[[-----------------------------------
Add Components
-------------------------------------]]
message("===========================================")
message("Components added:")
add_subdirectory(01_main/presenters/cli/integration)
add_subdirectory(01_main/controllers/demo_repl/integration)
message("===========================================")

#[[-----------------------------------
Main Executable
-------------------------------------]]
add_executable(MainApp
    01_main/entry/main.cc
)

target_link_libraries(MainApp PRIVATE
    cli_shell_lib
    demo_repl_lib
    # other_component_lib
)
