#===============================================================================
#           Project Source
#===============================================================================

INCLUDE_DIRECTORIES(.)
# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
AUX_SOURCE_DIRECTORY(parser SRC_LIST)
# directories
#ADD_SUBDIRECTORY(ast)
#ADD_SUBDIRECTORY(lexer)
#ADD_SUBDIRECTORY(parser)
#ADD_SUBDIRECTORY(generator)
#ADD_SUBDIRECTORY(logging)
#ADD_SUBDIRECTORY(sema)
#ADD_SUBDIRECTORY(interpreter)

# Source
SET(ADD_HEADERS
    ast/nodes.h
    ast/ops.h
    utilities/names.h
    utilities/object.h
    ast/magic.h 
    ast/constant.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    parser/parser.h
    parser/parsing_error.h
    lowering/lowering.h
    sema/sema.h
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
    ast/ops/context.cpp
    ast/ops/equality.cpp
    ast/ops/print.cpp
    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    parser/parser.cpp
    parser/parsing_error.cpp
    lowering/lowering.cpp
    sema/sema.cpp
    utilities/allocator.cpp 
    utilities/metadata.cpp
    utilities/pool.cpp
    utilities/object.cpp
    utilities/strings.cpp
    utilities/names.cpp
)

#SET (CUDA_SUPPORT
#    utilities/allocator.cu
#)

# Compile
#==================
# Compile as a library to avoid double compilation
# ADD_SUBDIRECTORY(cuda)
# ADD_SUBDIRECTORY(math)

# if not windows
#  stdc++fs
# endif

# FUNCTION(CLANG_STATIC_ANALYSIS target)
#     GET_TARGET_PROPERTY(SRCs ${target} SOURCES)
#     ADD_LIBRARY(${target}_analyze OBJECT EXCLUDE_FROM_ALL ${SRCs})
#     TARGET_INCLUDE_DIRECTORIES(
#         ${target}_analyze PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
#     )
#     SET_TARGET_PROPERTIES(${target}_analyze PROPERTIES
#                           COMPILE_OPTIONS "--analyze"
#                           EXCLUDE_FROM_DEFAULT_BUILD true)
# ENDFUNCTION()

# Support library for utilities
ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging spdlog::spdlog)


# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython spdlog::spdlog)

# Main interpreter loop
ADD_EXECUTABLE(${PROJECT_NAME} lython.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} spdlog::spdlog liblython liblogging)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

IF(UNIX)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j 4
        BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
        DEPENDS test)
ENDIF(UNIX)
