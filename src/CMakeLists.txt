# ===============================================================================
# Project Source
# ===============================================================================

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(../dependencies)

# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
AUX_SOURCE_DIRECTORY(parser SRC_LIST)


# directories
# ADD_SUBDIRECTORY(ast)
# ADD_SUBDIRECTORY(lexer)
# ADD_SUBDIRECTORY(parser)
# ADD_SUBDIRECTORY(generator)
# ADD_SUBDIRECTORY(logging)
# ADD_SUBDIRECTORY(sema)
# ADD_SUBDIRECTORY(interpreter)

# Source
SET(ADD_HEADERS
    ast/nodes.h
    ast/ops.h
    ast/values/native.h
    ast/values/generator.h

    utilities/names.h
    utilities/object.h
    ast/magic.h
    ast/constant.h
    builtin/operators.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    lexer/unlex.h
    parser/parser.h
    parser/parsing_error.h
    lowering/lowering.h
    sema/sema.h

    codegen/cpp/cpp_gen.h

    vm/tree.h

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

    dependencies/xx_hash.cpp

    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    lexer/unlex.cpp
    lowering/lowering.cpp

    parser/parser.cpp
    parser/parser_ext.cpp
    parser/parsing_error.cpp

    sema/sema.cpp
    sema/sema_import.cpp
    sema/errors.cpp
    sema/bindings.cpp
    sema/builtin.cpp

    codegen/cpp/cpp_gen.cpp

    vm/tree.cpp

    utilities/allocator.cpp
    utilities/metadata.cpp
    utilities/pool.cpp
    utilities/object.cpp
    utilities/strings.cpp
    utilities/names.cpp
)

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
INCLUDE_DIRECTORIES(mimalloc)


SET(LIBRARIES "")

IF(WITH_VALGRIND)
    MESSAGE(STATUS "Disabling mimalloc")
    ADD_DEFINITIONS(-DWITH_VALGRIND)
ELSE()
    LIST(APPEND LIBRARIES mimalloc-static)
ENDIF(WITH_VALGRIND)

IF(WITH_COZ)
    LIST(APPEND LIBRARIES dl)
    ADD_COMPILE_OPTIONS(-g3 -gdwarf-3)
    MESSAGE(STATUS "Compiling for coz")
ENDIF(WITH_COZ)

ADD_COMPILE_OPTIONS(-mavx2)
ADD_DEFINITIONS(-DWITH_LOG=${WITH_LOG})
ADD_DEFINITIONS(-DWITH_COZ=${WITH_COZ})

# Compile
# ==================

# Support library for utilities
ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging spdlog::spdlog)

# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython spdlog::spdlog ${LIBRARIES})

ADD_LIBRARY(stblyb stdlib/siphash.cpp)
SET_PROPERTY(TARGET stblyb PROPERTY CXX_STANDARD 20)

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
)
ADD_EXECUTABLE(lython ${ADD_CLI_SOURCE})
TARGET_LINK_LIBRARIES(lython spdlog::spdlog liblython liblogging argparse ${LIBRARIES})

SET_PROPERTY(TARGET lython PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
SET_PROPERTY(TARGET lython PROPERTY CMAKE_CXX_STANDARD 20)

IF(UNIX AND WITH_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest --timeout 10 --output-on-failure
        BASE_DIRECTORY "."
        DEPENDS test
    )
ENDIF()