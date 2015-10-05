project(lython)
cmake_minimum_required(VERSION 2.8)

# Enable C++11
# `llvm-config --cxxflags --ldflags --system-libs --libs core jit native`
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


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
    config.h
    Lexer/Token.h
    AbstractSyntaxTree/Operators.h
    path_hack.h
    ptr.h)


SET(ADD_SOURCE
    path_hack.cpp)

#===============================================================================
#           Compiling
#===============================================================================

#SET(LLVM_LINKING -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen
#                 -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86Desc -lLLVMObject
#                 -lLLVMMCParser -lLLVMBitReader -lLLVMX86Info -lLLVMX86AsmPrinter
#                 -lLLVMX86Utils -lLLVMJIT -lLLVMExecutionEngine -lLLVMCodeGen
#                 -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils
#                 -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMMC -lLLVMCore
#                 -lLLVMSupport -lrt -ldl -ltinfo -lpthread -lz)

# testing
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
