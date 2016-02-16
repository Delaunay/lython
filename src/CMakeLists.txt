#===============================================================================
#           Project Source
#===============================================================================

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Sources
# aux_source_directory("${PROJECT_SOURCE_DIR}/AbstractSyntaxTree" SRC_LIST)
#AUX_SOURCE_DIRECTORY("${PROJECT_SOURCE_DIR}/Lexer"              SRC_LIST)
# aux_source_directory("${PROJECT_SOURCE_DIR}/Parser"             SRC_LIST)
# aux_source_directory("${PROJECT_SOURCE_DIR}/Generator"          SRC_LIST)

# directories
ADD_SUBDIRECTORY(AbstractSyntaxTree)
ADD_SUBDIRECTORY(Lexer)
ADD_SUBDIRECTORY(Parser)
ADD_SUBDIRECTORY(Generator)

# Source
SET(ADD_HEADERS
    AbstractSyntaxTree/Expressions.h
    AbstractSyntaxTree/Names.h
    # AbstractSyntaxTree/Types.h
    Lexer/Lexer.h
    # Lexer/PreLexer.h
    Lexer/Buffer.h
    Lexer/Tokens.h
    Parser/Parser.h
    fmt.h
)

SET(ADD_SOURCE
    AbstractSyntaxTree/Expressions.cpp
    AbstractSyntaxTree/Names.cpp
    # AbstractSyntaxTree/Types.cpp
    Lexer/Lexer.cpp
    # Lexer/PreLexer.cpp
    Lexer/Buffer.cpp
    Lexer/Tokens.cpp
    Parser/Parser.cpp
)

# Compile
#==================
# Compile as a library to avoid double compilation
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})

ADD_EXECUTABLE(${PROJECT_NAME} lython.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} liblython)

# Enable C++11
SET(${CXX_STANDARD_REQUIRED} ON)
SET_PROPERTY(TARGET liblython PROPERTY CXX_STANDARD 11)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)




