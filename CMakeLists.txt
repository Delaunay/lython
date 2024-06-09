# required for C++11 features
CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

# project name + language used
PROJECT(lython C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# debug/release
SET(CMAKE_RELEASE_POSTFIX "")
SET(CMAKE_DEBUG_POSTFIX "-debug")

# Compiler-flag
IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    MESSAGE(STATUS "Building Debug Version")

    IF(UNIX)
        ADD_DEFINITIONS(-O0)
        ADD_LINK_OPTIONS(-rdynamic)
    ENDIF(UNIX)
ELSE()
    MESSAGE(STATUS "Building Release Version: " ${CMAKE_BUILD_TYPE})

    # causes: nvcc fatal   : redefinition of argument 'optimize'
    # ADD_DEFINITIONS(-O2)
ENDIF()

# specify where compiled files must go
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# add cmake FindXXX script to find pre-compiled libraries if any
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/binaries/")
MESSAGE(STATUS ${CMAKE_MODULE_PATH})
INCLUDE(constant)

# Project's Options
# ====================================
OPTION(BUILD_TESTING "Enable tests" ON)
OPTION(BUILD_USING_CLANG "Enable with clang" OFF)
OPTION(BUILD_BENCHMARKS "Build Benchmarks" ON)
OPTION(BUILD_DOCUMENTATION "Build docs" OFF)
OPTION(BUILD_DOXYGEN "Build Doxygen docs" ON)
OPTION(BUILD_SPHINX "Build Sphinx docs" ON)
OPTION(CLANG_TIDY "Run clang-tidy" OFF)
OPTION(WITH_VALGRIND "Disable mimalloc for better valgrind support" OFF)
OPTION(WITH_COVERAGE "Enable coverage generation" ON)
OPTION(WITH_LOG "Enable compiler log" ON)
OPTION(WITH_COZ "Enable coz profiler" OFF)
OPTION(NO_LLVM "Disable LLVM" OFF)

IF(BUILD_USING_CLANG)
    IF(WITH_COVERAGE)
        ADD_COMPILE_OPTIONS(-fprofile-instr-generate -fcoverage-mapping)
        LINK_LIBRARIES(-fprofile-instr-generate -fcoverage-mapping)
    ENDIF()
ENDIF()

# Identify Targets
SET(BUILD_WEBASSEMBLY 0)
SET(BUILD_POSIX 0)
SET(BUILD_WINDOWS 0)

IF(CMAKE_CXX_COMPILER MATCHES "/em.*$")
    MESSAGE(" * Platform: Emscripten")
    SET(BUILD_WEBASSEMBLY 1)

    # Disable not supported targets
    SET(BUILD_TESTING OFF) #
    SET(BUILD_BENCHMARKS OFF) #

    # Disable not supported features
    SET(WITH_COZ 0)
    SET(WITH_COVERAGE 0)
    SET(WITH_LOG 0)
    SET(WITH_VALGRIND 1)
ENDIF()

IF(UNIX)
    MESSAGE(" * Platform: Unix-Like")
    SET(BUILD_POSIX 1)
    SET(WITH_LOG 1)

# export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1
# export UBSAN_OPTIONS=abort_on_error=1:halt_on_error=1
# -fsanitize=address,undefined,thread -fsanitize-trap
# -g3                                                       # Debug information

# -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion
ELSEIF(WIN32)
    MESSAGE(" * Platform: Windows")
    SET(BUILD_WINDOWS 1)
    add_compile_options(/EHsc)
    add_compile_options(/Z7)

    # /RTCcsu
    # /W4 /wd4146 /wd4245 /D_CRT_SECURE_NO_WARNINGS
    # /Z7                                            <= Debug information embedded in binaries

    # Disable not supported features
    SET(WITH_COZ 0)
    SET(WITH_COVERAGE 0)
    SET(WITH_LOG 1)
ENDIF()

# Binary/pre-compiled Dependencies
# ====================================

# C++20 does not work for gcc & fmt as of Nov - 2022
SET(LY_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD ${LY_CXX_STANDARD})
SET(CXX_STANDARD_REQUIRED ON)

# FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Git REQUIRED)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(argparse)
FIND_PACKAGE(mimalloc)
FIND_PACKAGE(fmt REQUIRED)
FIND_PACKAGE(Catch2 REQUIRED)
# FIND_PACKAGE(backtrace)


# CMake Exentions
# ====================================

# Find revision ID and hash of the sourcetree
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE(genrev)

MESSAGE(STATUS ${CMAKE_BUILD_TYPE} " " ${CMAKE_CONFIGURATION_TYPES})
SET(LLVM_REQUESTED_BUILD_TYPE ${CMAKE_BUILD_TYPE})
INCLUDE(ConfigureLLVM)

IF(UNIX AND WITH_COVERAGE)
    INCLUDE(CodeCoverage)
    append_coverage_compiler_flags()
    SET(COVERAGE_EXCLUDES "${PROJECT_SOURCE_DIR}/dependencies/*")
ENDIF()

IF(CLANG_TIDY)
    FILE(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
    SET(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
    SET(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,-clang-analyzer-cplusplus*")
    UNSET(clang_tidy_sha1)
    SET(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/src/*'")
ENDIF(CLANG_TIDY)

# Subdirectories
# ====================================

# Dependencies to be compiled
ADD_SUBDIRECTORY(dependencies)

# How to compile main libs
ADD_SUBDIRECTORY(src)

# Tests and Coverage
IF(BUILD_TESTING)
    ENABLE_TESTING()
    MESSAGE(STATUS "Building tests")
    ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)

IF(BUILD_BENCHMARKS)
    MESSAGE(STATUS "Building benchmarks")
    ADD_SUBDIRECTORY(benchmarks)
ENDIF(BUILD_BENCHMARKS)

# IF (BUILD_BENCHMARK)
# MESSAGE(STATUS "Building Benchmarks")
# ADD_SUBDIRECTORY(benchmark)
# ENDIF(BUILD_BENCHMARK)

# Gen doc ?
# IF (BUILD_DOCUMENTATION)
# MESSAGE(STATUS "Documentation building is enabled")
# ADD_SUBDIRECTORY(docs)
# ENDIF (BUILD_DOCUMENTATION)

# Helper commands
# ====================================
# so you do not have to leave the command line
# or remember the exact arguments
ADD_CUSTOM_TARGET(enable-examples
    cmake -DBUILD_EXAMPLES=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-tidy
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCLANG_TIDY=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-compiledb
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(disable-examples
    cmake -DBUILD_EXAMPLES=OFF ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-release
    cmake -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-debug
    cmake -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-test
    cmake -DBUILD_TESTING=ON ${CMAKE_SOURCE_DIR})

# Resource Files
# ====================================
# Some IDEs need the files to be inside a cmake target to be displayed
ADD_CUSTOM_TARGET(zothers ALL SOURCES
    README.rst
    .gitignore
    .gitmodules
    .github/workflows/tests.yml
)
