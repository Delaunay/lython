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
    ast/expressions.h
    ast/nodes.h
    ast/names.h
    ast/visitor.h
    ast/print.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    parser/parser.h
    parser/module.h
    utilities/optional.h
    utilities/pool.h
    sema/sema.h
    utilities/trie.h
    utilities/stack.h
    utilities/allocator.h
    utilities/metadata.h
    utilities/stopwatch.h
    utilities/strings.h
    utilities/guard.h
    interpreter/interpreter.h
    interpreter/value.h
    dtypes.h
    "${BUILDDIR}/revision_data.h"
)

SET(ADD_SOURCE
    ast/expressions.cpp
    ast/names.cpp
    ast/print.cpp
    ast/nodes.cpp
    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    parser/parser.cpp
    parser/declarations.cpp
    parser/import.cpp
    parser/struct.cpp
    sema/sema.cpp
    utilities/allocator.cpp
    utilities/metadata.cpp
    utilities/pool.cpp
    utilities/strings.cpp
    interpreter/interpreter.cpp
    interpreter/value.cpp
)

#SET (CUDA_SUPPORT
#    utilities/allocator.cu
#)

# Compile
#==================
# Compile as a library to avoid double compilation
# ADD_SUBDIRECTORY(cuda)
# ADD_SUBDIRECTORY(math)

# Support library for utilities
ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging spdlog::spdlog stdc++fs)

# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython spdlog::spdlog stdc++fs)

# Main interpreter loop
ADD_EXECUTABLE(${PROJECT_NAME} lython.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} spdlog::spdlog stdc++fs liblython liblogging)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

IF(UNIX)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j 4
        BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
        DEPENDS test)
ENDIF(UNIX)
