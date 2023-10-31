# ===============================================================================
# Project Source
# ===============================================================================

# Includes
# ========

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(../dependencies/spdlog/include)
INCLUDE_DIRECTORIES(../dependencies)
INCLUDE_DIRECTORIES(mimalloc)

# Libraries
# =========
SET(LIBRARIES "")

IF(WITH_VALGRIND)
    MESSAGE(STATUS "Disabling mimalloc")
    ADD_DEFINITIONS(-DWITH_VALGRIND)
ELSE()
    LIST(APPEND LIBRARIES mimalloc-static)
ENDIF(WITH_VALGRIND)

IF(WITH_LOG)
    LIST(APPEND LIBRARIES spdlog::spdlog)
ENDIF(WITH_LOG)

IF(WITH_COZ)
    LIST(APPEND LIBRARIES dl)
    ADD_COMPILE_OPTIONS(-g3 -gdwarf-3)
    MESSAGE(STATUS "Compiling for coz")
ENDIF(WITH_COZ)

# Source
SET(ADD_HEADERS
    ast/nodes.h
    ast/ops.h
    ast/values/native.h
    ast/values/generator.h
    ast/magic.h
    ast/constant.h
    builtin/operators.h
    compatibility/compatibility.h
    codegen/cpp/cpp_gen.h
    codegen/clang/clang_gen.h
    codegen/llvm/llvm_gen.h
    codegen/llvm/llvm_jit.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    lexer/unlex.h
    lowering/lowering.h
    parser/parser.h
    parser/parsing_error.h
    parser/format_spec.h
    sema/sema.h
    vm/tree.h
    vm/garbage_collector.h
    utilities/names.h
    utilities/object.h
    utilities/optional.h
    utilities/pool.h
    utilities/trie.h
    utilities/stack.h
    utilities/allocator.h
    utilities/metadata.h
    utilities/stopwatch.h
    utilities/strings.h
    utilities/guard.h
    dtypes.h
    "${BUILDDIR}/revision_data.h"
)

SET(ADD_SOURCE
    ast/nodes.cpp
    ast/values/native.cpp
    ast/values/generator.cpp
    ast/ops/context.cpp
    ast/ops/equality.cpp
    ast/ops/attribute.cpp
    ast/ops/print.cpp
    ast/ops/circle.cpp
    builtin/operators.cpp
    codegen/cpp/cpp_gen.cpp
    codegen/clang/clang_gen.cpp
    codegen/llvm/llvm_gen.cpp
    codegen/llvm/llvm_jit.cpp
    codegen/llvm/llvm_emit.cpp
    dependencies/xx_hash.cpp
    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    lexer/unlex.cpp
    lowering/lowering.cpp
    parser/parser.cpp
    parser/parser_ext.cpp
    parser/parsing_error.cpp
    parser/format_spec.cpp
    printer/error_printer.cpp
    sema/sema.cpp
    sema/sema_import.cpp
    sema/errors.cpp
    sema/bindings.cpp
    sema/builtin.cpp
    vm/tree.cpp
    vm/garbage_collector.cpp
    utilities/allocator.cpp
    utilities/metadata.cpp
    utilities/pool.cpp
    utilities/object.cpp
    utilities/strings.cpp
    utilities/names.cpp
)

# Configure Platforms
# ==============================================
IF(BUILD_WINDOWS)
ENDIF()

IF(BUILD_POSIX)
    ADD_COMPILE_OPTIONS(-mavx2)
ENDIF()

IF(BUILD_WEBASSEMBLY)
    add_definitions("-s LINKABLE=1")
    add_definitions("-s EXPORT_ALL=1")
ENDIF()

# Defintions
ADD_DEFINITIONS(-DBUILD_WEBASSEMBLY=${BUILD_WEBASSEMBLY})
ADD_DEFINITIONS(-DBUILD_POSIX=${BUILD_POSIX})
ADD_DEFINITIONS(-DBUILD_WINDOWS=${BUILD_WINDOWS})
ADD_DEFINITIONS(-DWITH_LLVM=${WITH_LLVM})
ADD_DEFINITIONS(-DWITH_LLVM_CODEGEN=1)
ADD_DEFINITIONS(-DWITH_CLANG_CODEGEN=0)

# ADD_DEFINITIONS(-DWIN32=${WIN32})
ADD_DEFINITIONS(-DWITH_LOG=${WITH_LOG})
ADD_DEFINITIONS(-DWITH_COZ=${WITH_COZ})
ADD_DEFINITIONS(-DFMT_USE_CONSTEXPR=0)

# Compile
# ==================

# Support library for utilities
ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging ${LIBRARIES})

# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython ${LIBRARIES} ${LLVM_LIRARIES})
ADD_DEPENDENCIES(liblython zlib zlibstatic)

ADD_LIBRARY(stblyb stdlib/siphash.cpp)
SET_PROPERTY(TARGET stblyb PROPERTY CXX_STANDARD ${LY_CXX_STANDARD})

# CLI
SET(ADD_CLI_SOURCE
    cli/cli.cpp
    cli/commands/code.cpp
    cli/commands/format.cpp
    cli/commands/codegen.cpp
    cli/commands/debug.cpp
    cli/commands/doc.cpp
    cli/commands/install.cpp
    cli/commands/internal.cpp
    cli/commands/linter.cpp
    cli/commands/profile.cpp
    cli/commands/tests.cpp
    cli/commands/vm.cpp
)
ADD_EXECUTABLE(lython ${ADD_CLI_SOURCE})
TARGET_LINK_LIBRARIES(lython liblython liblogging argparse ${LIBRARIES} ${LLVM_LIRARIES})

SET_PROPERTY(TARGET lython PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
SET_PROPERTY(TARGET lython PROPERTY CMAKE_CXX_STANDARD ${LY_CXX_STANDARD})

IF(UNIX AND WITH_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest --timeout 10 --output-on-failure
        BASE_DIRECTORY "."
        DEPENDS test
    )
ENDIF()

# SET (CUDA_SUPPORT
# utilities/allocator.cu
# )

# Compile as a library to avoid double compilation
# ADD_SUBDIRECTORY(cuda)
# ADD_SUBDIRECTORY(math)

# if not windows
# stdc++fs
# endif

# FUNCTION(CLANG_STATIC_ANALYSIS target)
# GET_TARGET_PROPERTY(SRCs ${target} SOURCES)
# ADD_LIBRARY(${target}_analyze OBJECT EXCLUDE_FROM_ALL ${SRCs})
# TARGET_INCLUDE_DIRECTORIES(
# ${target}_analyze PUBLIC
# $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
# )
# SET_TARGET_PROPERTIES(${target}_analyze PROPERTIES
# COMPILE_OPTIONS "--analyze"
# EXCLUDE_FROM_DEFAULT_BUILD true)
# ENDFUNCTION()

# find_package(backtrace)

add_subdirectory(tide)




# Plugin Example
ADD_LIBRARY(PluginExample SHARED plugin/example.cpp)
