# Orchestrator for FindCHOLMOD.cmake hermetic unit tests.
#
# Generates fake CHOLMOD installations covering each OpenMP-detection scenario
# and configures test/detect against each one, asserting the result. Requires
# only CMake -- no compiler and no SuiteSparse installation.
#
#   cmake -S test -B build-unit
#   ctest --test-dir build-unit --output-on-failure
cmake_minimum_required(VERSION 3.16)
project(findcholmod_unit_tests LANGUAGES NONE)

enable_testing()

set(_findcholmod_dir "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_detect_src "${CMAKE_CURRENT_SOURCE_DIR}/detect")
set(_fixtures "${CMAKE_CURRENT_BINARY_DIR}/fixtures")

# Write a fake CHOLMOD install: a cholmod.h carrying the given OpenMP line, plus
# the libraries find_library() must resolve. The libraries are tiny stub files
# whose *contents* feed the file(STRINGS) fallback path. One file is written per
# find_library suffix so the stubs are located on every platform.
function(_make_fixture name header_line lib_content)
    set(_prefix "${_fixtures}/${name}")
    set(_inc "${_prefix}/include/suitesparse")
    set(_libdir "${_prefix}/lib")
    file(REMOVE_RECURSE "${_prefix}")
    file(MAKE_DIRECTORY "${_inc}" "${_libdir}")

    file(
        WRITE "${_inc}/cholmod.h"
        "/* fake cholmod.h (unit test fixture: ${name}) */\n"
        "#define CHOLMOD_MAIN_VERSION 5\n"
        "#define CHOLMOD_SUB_VERSION 3\n"
        "#define CHOLMOD_SUBSUB_VERSION 1\n"
        "${header_line}\n"
    )

    list(GET CMAKE_FIND_LIBRARY_PREFIXES 0 _p)
    foreach(
        _comp
        cholmod
        amd
        camd
        colamd
        ccolamd
        suitesparseconfig
    )
        foreach(_s IN LISTS CMAKE_FIND_LIBRARY_SUFFIXES)
            file(WRITE "${_libdir}/${_p}${_comp}${_s}" "${lib_content}")
        endforeach()
    endforeach()
endfunction()

function(_add_detect_test name expect)
    add_test(
        NAME omp_detect.${name}
        COMMAND
            ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -S "${_detect_src}" -B
            "${CMAKE_CURRENT_BINARY_DIR}/detect-${name}" -DFINDCHOLMOD_DIR=${_findcholmod_dir}
            -DFAKE_PREFIX=${_fixtures}/${name} -DEXPECT_OPENMP=${expect}
    )
endfunction()

# 1. Modern SuiteSparse built WITH OpenMP -> macro present in cholmod.h.
_make_fixture(header_on "#define CHOLMOD_HAS_OPENMP" "")
_add_detect_test(header_on TRUE)

# 2. Modern SuiteSparse built WITHOUT OpenMP -> macro #undef'd, no library refs.
_make_fixture(header_off "/* #undef CHOLMOD_HAS_OPENMP */" "")
_add_detect_test(header_off FALSE)

# 3. Legacy SuiteSparse (no macro) linked against OpenMP -> file(STRINGS) finds
#    the runtime dependency / symbols embedded in the library.
_make_fixture(legacy_on "" "stub library\n@rpath/libgomp.so.1\nGOMP_parallel\n")
_add_detect_test(legacy_on TRUE)

# 4. Legacy SuiteSparse (no macro) without OpenMP -> nothing matches.
_make_fixture(legacy_off "" "stub library with no openmp references\n")
_add_detect_test(legacy_off FALSE)
