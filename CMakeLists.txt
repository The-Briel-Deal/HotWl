cmake_minimum_required(VERSION 3.5)
project(gfwl)

# Set the source files.
set(SOURCES
  src/main.c
  src/layer_shell.c
)

include_directories(
  .
  "src/"
  "/usr/include/wlroots-0.18"
  "/usr/include/pixman-1"
  "/usr/include/libdrm"
)

execute_process(
    OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${protoName}.cpp
           ${CMAKE_SOURCE_DIR}/protocols/${protoName}.hpp
    COMMAND hyprwayland-scanner ${path}/${protoName}.xml ${CMAKE_SOURCE_DIR}/protocols/
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
# Setup output directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

add_compile_definitions(WLR_USE_UNSTABLE)

add_executable(test_binary ${SOURCES})
