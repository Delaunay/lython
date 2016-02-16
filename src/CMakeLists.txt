project(lython)
cmake_minimum_required(VERSION 2.8)

#===============================================================================
#           Project Source
#===============================================================================

# Sources
aux_source_directory("${PROJECT_SOURCE_DIR}/AbstractSyntaxTree" SRC_LIST)
aux_source_directory("${PROJECT_SOURCE_DIR}/Lexer"              SRC_LIST)
aux_source_directory("${PROJECT_SOURCE_DIR}/Parser"             SRC_LIST)
aux_source_directory("${PROJECT_SOURCE_DIR}/Generator"          SRC_LIST)

# directories
add_subdirectory("${PROJECT_SOURCE_DIR}/AbstractSyntaxTree")
add_subdirectory("${PROJECT_SOURCE_DIR}/Lexer")
add_subdirectory("${PROJECT_SOURCE_DIR}/Parser")
add_subdirectory("${PROJECT_SOURCE_DIR}/Generator")

# Source
SET(ADD_HEADERS
    AbstractSyntaxTree/Expressions.h
    AbstractSyntaxTree/Names.h
    # AbstractSyntaxTree/Types.h
    Lexer/Lexer.h
    Lexer/PreLexer.h
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
    Lexer/PreLexer.cpp
    Lexer/Buffer.cpp
    Lexer/Tokens.cpp
    Parser/Parser.cpp
)

#==================
add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/main.cpp"
                                ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})

target_link_libraries(${PROJECT_NAME} ${LLVM_LINKING})


## default Compiler
##==================
#add_executable(ilython "${PROJECT_SOURCE_DIR}/interpreter/interpreter.cpp"
#                        ${SRC_LIST} ${LEXER_HEADERS})

#target_link_libraries(ilython ${LLVM_LINKING})


## Compiling Original Source Code! (Check is all flags are here)
##==================
#add_executable(full_source "${PROJECT_SOURCE_DIR}/test/full_source.cpp")
#target_link_libraries(full_source ${LLVM_LINKING})
