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
    ast/fold.h
    ast/visitor.h
    ast/print.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    parser/parser.h
    parser/module.h
    utilities/optional.h
    fmt.h
    sema/sema.h
    utilities/trie.h
    utilities/stack.h
    utilities/allocator.h
    utilities/metadata.h
    interpreter/interpreter.h
    interpreter/value.h
    interpreter/value.cpp
    Types.h
    "${BUILDDIR}/revision_data.h"
)

SET(ADD_SOURCE
    ast/expressions.cpp
    ast/names.cpp
    ast/print.cpp
    ast/nodes.cpp
    ast/fold.cpp
    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    parser/parser.cpp
    parser/declarations.cpp
    sema/sema.cpp
    utilities/allocator.cpp
    utilities/metadata.cpp
    interpreter/interpreter.cpp
)

# Compile
#==================
# Compile as a library to avoid double compilation

# Support library for utilities
ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging fmt::fmt spdlog::spdlog)

# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython fmt::fmt)

# Main interpreter loop
ADD_EXECUTABLE(${PROJECT_NAME} lython.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} fmt::fmt liblython liblogging)
