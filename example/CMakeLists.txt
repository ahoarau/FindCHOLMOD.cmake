cmake_minimum_required(VERSION 3.16)
project(cholmod_example LANGUAGES C)

# Make the parent directory's FindCHOLMOD.cmake discoverable.
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_package(CHOLMOD REQUIRED)

add_executable(cholmod_demo main.c)
target_link_libraries(cholmod_demo PRIVATE SuiteSparse::CHOLMOD)

enable_testing()
add_test(NAME cholmod_demo COMMAND cholmod_demo)
