# Hermetic unit test for FindCHOLMOD's OpenMP detection.
#
# No compiler and no real SuiteSparse are required: a fake install is supplied
# via -DFAKE_PREFIX and the module is driven through its manual (non-config)
# path. The orchestrator in ../CMakeLists.txt configures this project once per
# scenario; a failed assertion aborts configuration and fails the ctest case.
cmake_minimum_required(VERSION 3.16)
project(cholmod_omp_detect LANGUAGES NONE)

foreach(_required FINDCHOLMOD_DIR FAKE_PREFIX EXPECT_OPENMP)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "test driver: -D${_required} must be provided")
    endif()
endforeach()

list(PREPEND CMAKE_MODULE_PATH "${FINDCHOLMOD_DIR}")

# Restrict discovery to the fake prefix so the host's real CHOLMOD (a config
# file or libraries) can never be picked up. This also guarantees the manual
# fallback path runs instead of the config short-circuit.
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
set(CMAKE_FIND_USE_INSTALL_PREFIX FALSE)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY FALSE)
set(CMAKE_PREFIX_PATH "${FAKE_PREFIX}")

# Stand-in dependency targets so the module does not call find_package() for
# OpenMP/BLAS/LAPACK (which would need a real toolchain). The module only
# searches when the corresponding target does not already exist.
add_library(OpenMP::OpenMP_C INTERFACE IMPORTED)
add_library(BLAS::BLAS INTERFACE IMPORTED)
add_library(LAPACK::LAPACK INTERFACE IMPORTED)

find_package(CHOLMOD REQUIRED)

if(NOT CHOLMOD_FOUND)
    message(FATAL_ERROR "FindCHOLMOD did not find the fake install at ${FAKE_PREFIX}")
endif()

get_target_property(_ssc_links SuiteSparse::SuiteSparseConfig INTERFACE_LINK_LIBRARIES)
if(_ssc_links AND "OpenMP::OpenMP_C" IN_LIST _ssc_links)
    set(_openmp_linked TRUE)
else()
    set(_openmp_linked FALSE)
endif()

if(EXPECT_OPENMP)
    if(NOT CHOLMOD_HAS_OPENMP)
        message(
            FATAL_ERROR
            "FAIL: expected CHOLMOD_HAS_OPENMP=TRUE but got '${CHOLMOD_HAS_OPENMP}'"
        )
    endif()
    if(NOT _openmp_linked)
        message(
            FATAL_ERROR
            "FAIL: OpenMP::OpenMP_C not propagated via SuiteSparse::SuiteSparseConfig "
            "(INTERFACE_LINK_LIBRARIES='${_ssc_links}')"
        )
    endif()
else()
    if(CHOLMOD_HAS_OPENMP)
        message(FATAL_ERROR "FAIL: expected CHOLMOD_HAS_OPENMP=FALSE but got TRUE")
    endif()
    if(_openmp_linked)
        message(
            FATAL_ERROR
            "FAIL: OpenMP::OpenMP_C linked even though CHOLMOD has no OpenMP support"
        )
    endif()
endif()

message(STATUS "PASS: CHOLMOD_HAS_OPENMP=${CHOLMOD_HAS_OPENMP} openmp_linked=${_openmp_linked}")
