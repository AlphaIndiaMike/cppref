#[[-----------------------------------
CLI Shell Component
-------------------------------------]]
message("  → CLI Shell component")

add_library(cli_shell_lib STATIC
    ../src/cli_shell.cc
)

target_include_directories(cli_shell_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
)
