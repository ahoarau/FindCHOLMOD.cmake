cmake_minimum_required(VERSION 3.16)
project(cholmod_quiet_test LANGUAGES NONE)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_package(CHOLMOD QUIET)

if(CHOLMOD_FOUND)
    message(
        FATAL_ERROR
        "FindCHOLMOD: CHOLMOD was found but this environment has no CHOLMOD "
        "installed — the test environment is broken."
    )
endif()

message(STATUS "FindCHOLMOD QUIET: correctly reported not found (CHOLMOD_FOUND=FALSE)")
