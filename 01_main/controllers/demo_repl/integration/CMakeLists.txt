set(COMPONENT_NAME "demo_repl")

#[[-----------------------------------
Demo Controller REPL
-------------------------------------]]
message("  → ${COMPONENT_NAME} ")

add_library(${COMPONENT_NAME}_lib STATIC
    #../src/${COMPONENT_NAME}.c
    ../src/demo_controller.cc
)

# Link to cli_shell_lib to access its PUBLIC include directories
target_link_libraries(${COMPONENT_NAME}_lib PUBLIC
    cli_shell_lib
)

target_include_directories(${COMPONENT_NAME}_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
)

if(PROJECT_SHOW_CMAKE_DEBUG_INFO)
    get_target_property(DBG_INCLUDES ${COMPONENT_NAME}_lib INTERFACE_INCLUDE_DIRECTORIES)
    message("${COMPONENT_NAME} includes: ${DBG_INCLUDES}")

    get_target_property(DBG_LINK_LIBS ${COMPONENT_NAME}_lib LINK_LIBRARIES)
    message("${COMPONENT_NAME} linked libraries: ${DBG_LINK_LIBS}")
endif()
