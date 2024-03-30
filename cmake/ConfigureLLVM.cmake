if(WIN32)
    # SET(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/binaries/lib/zlib.lib)
    # SET(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/binaries/include)
    
    SET(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/zlib)

    IF(${LLVM_REQUESTED_BUILD_TYPE} MATCHES "Debug")
        MESSAGE(STATUS "Debug: " ${CMAKE_BINARY_DIR})
        SET(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/lib/${LLVM_REQUESTED_BUILD_TYPE}/zlibstaticd.lib)
        SET(LLVM_PATH K:/llvm-build/Debug)
        SET(LLVM_DIR K:/llvm-build/Debug/lib/cmake/llvm)
        SET(Clang_DIR K:/llvm-build/Debug/lib/cmake/clang)
    ELSE()
        SET(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/lib/${LLVM_REQUESTED_BUILD_TYPE}/zlibstatic.lib) 
        SET(LLVM_PATH K:/llvm-build/Release)
        SET(LLVM_DIR K:/llvm-build/Release/lib/cmake/llvm)
        SET(Clang_DIR K:/llvm-build/Release/lib/cmake/clang)
    ENDIF() 

else()
    SET(ZLIB_INCLUDE_DIR /usr/lib/x86_64-linux-gnu)
    find_package(ZLIB)
endif(WIN32)

# SET(NO_LLVM 1)


IF(NO_LLVM)
    SET(WITH_LLVM 0)
    SET(LLVM_LIRARIES)
    MESSAGE(STATUS "LLVM disabled")
ELSE()
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
        
        IF(${LLVM_REQUESTED_BUILD_TYPE} MATCHES "Debug")
            link_directories(K:/llvm-build/Debug/Debug/lib)
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            link_directories(K:/llvm-build/RelWithDebInfo/RelWithDebInfo/lib)
        ELSE()
            link_directories(${LLVM_LIBRARY_DIRS})
        ENDIF()

        llvm_map_components_to_libnames(LLVM_LIRARIES
            # Base
            support core irreader X86
            # ORC JiT
            Analysis ExecutionEngine InstCombine  Object 
            OrcJIT RuntimeDyld ScalarOpts native

            # MC
            target

        )
        MESSAGE(STATUS "${LLVM_LIRARIES}")
    ELSE(LLVM_FOUND)
        SET(WITH_LLVM 0)
        SET(LLVM_LIRARIES)
        MESSAGE(STATUS "LLVM not found!")
    ENDIF(LLVM_FOUND)
ENDIF()