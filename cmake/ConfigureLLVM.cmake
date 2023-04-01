if(WIN32)
    SET(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/binaries/lib/zlib.lib)
    SET(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/binaries/include)

    SET(LLVM_PATH E:/llvm-install)
    SET(LLVM_DIR E:/llvm-install/lib/cmake/llvm)
    SET(Clang_DIR E:/llvm-install/lib/cmake/clang)
else()
    SET(ZLIB_INCLUDE_DIR /usr/lib/x86_64-linux-gnu)
    find_package(ZLIB)
endif(WIN32)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)

# link_directories(${CMAKE_SOURCE_DIR}/binaries/lib)
# include_directories(${CMAKE_SOURCE_DIR}/binaries/include)
IF(LLVM_FOUND)
    ADD_DEFINITIONS(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING=1)
    SET(WITH_LLVM 1)

    # Putting system here prevents the compiler from printing
    # warnings inside the headers
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    link_directories(${LLVM_LIBRARY_DIRS})

    llvm_map_components_to_libnames(LLVM_LIRARIES support core irreader X86)
    MESSAGE(STATUS "${LLVM_LIRARIES}")
ELSE(LLVM_FOUND)
    SET(WITH_LLVM 0)
    SET(LLVM_LIRARIES)
    MESSAGE(STATUS "LLVM not found!")
ENDIF(LLVM_FOUND)
