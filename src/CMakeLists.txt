#===============================================================================
#           Project Source
#===============================================================================

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Sources
# aux_source_directory("${PROJECT_SOURCE_DIR}/AbstractSyntaxTree" SRC_LIST)
#AUX_SOURCE_DIRECTORY("${PROJECT_SOURCE_DIR}/Lexer"              SRC_LIST)
AUX_SOURCE_DIRECTORY(parser SRC_LIST)
# aux_source_directory("${PROJECT_SOURCE_DIR}/Generator"          SRC_LIST)

# directories
ADD_SUBDIRECTORY(ast)
ADD_SUBDIRECTORY(lexer)
ADD_SUBDIRECTORY(parser)
ADD_SUBDIRECTORY(generator)
ADD_SUBDIRECTORY(logging)
ADD_SUBDIRECTORY(sema)
ADD_SUBDIRECTORY(interpreter)

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
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython fmt::fmt)

ADD_LIBRARY(liblogging logging/logging.cpp logging/logging.h)
TARGET_LINK_LIBRARIES(liblogging fmt::fmt)


ADD_EXECUTABLE(${PROJECT_NAME} lython.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} fmt::fmt liblython liblogging)

# Enable C++11
SET(${CXX_STANDARD_REQUIRED} ON)
SET_PROPERTY(TARGET liblython PROPERTY CXX_STANDARD 20)
SET_PROPERTY(TARGET liblogging PROPERTY CXX_STANDARD 20)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)





