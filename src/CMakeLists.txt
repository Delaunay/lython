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
    ast/names.h
    ast/value_holders.h
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
)

SET(ADD_SOURCE
    ast/expressions.cpp
    ast/names.cpp
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

setup_target_for_coverage_gcovr_html(
    NAME coverage
    EXECUTABLE ctest -j 4
    BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
    DEPENDS test)

