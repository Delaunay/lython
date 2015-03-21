project(lython)
cmake_minimum_required(VERSION 2.8)

# Enable C++11

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O1 -O2 -O3")


#===============================================================================
#           LLVM Source
#===============================================================================

SET(LLVM_DIR "~/llvm/build/")
SET(LLVM_BUILD_DIR "~/llvm/")

include_directories("${LLVM_DIR}/include")
include_directories("${LLVM_BUILD_DIR}/include")

#===============================================================================
#           Project Source
#===============================================================================

# Home Source
aux_source_directory(. SRC_LIST)

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

SET(LEXER_HEADERS
    Lexer/Token.h
    AbstractSyntaxTree/Operators.h)

#===============================================================================
#           Compiling
#===============================================================================

add_executable(${PROJECT_NAME} ${SRC_LIST} ${LEXER_HEADERS})

target_link_libraries(${PROJECT_NAME} libLLVMCore.a libLLVMSupport.a
                        -ldl        # Dynamic loading
                        -pthread
                        -ltinfo)    # Terminal Information
