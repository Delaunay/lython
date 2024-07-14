if(WIN32)
    # SET(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/binaries/lib/zlib.lib)
    # SET(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/binaries/include)

    # SET(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/zlib)
    # SET(ZLIB_LIBRARY_DEB ${CMAKE_BINARY_DIR}/lib/Debug/zlibstaticd.lib)
    # SET(ZLIB_LIBRARY_DEV ${CMAKE_BINARY_DIR}/lib/RelWithDebInfo/zlibstatic.lib)
    # SET(ZLIB_LIBRARY_REL ${CMAKE_BINARY_DIR}/lib/Release/zlibstatic.lib)

    SET(LLVM_PATH_DEB K:/llvm-build/Debug/Debug)
    SET(LLVM_DIR_DEB K:/llvm-build/Debug/lib/cmake/llvm)
    SET(Clang_DIR_DEB K:/llvm-build/Debug/lib/cmake/clang)

    SET(LLVM_PATH_DEV K:/llvm-build/RelWithDebInfo/RelWithDebInfo)
    SET(LLVM_DIR_DEV K:/llvm-build/RelWithDebInfo/lib/cmake/llvm)
    SET(Clang_DIR_DEV K:/llvm-build/RelWithDebInfo/lib/cmake/clang)

    SET(LLVM_PATH_REL K:/llvm-build/Release/Release)
    SET(LLVM_DIR_REL K:/llvm-build/Release/lib/cmake/llvm)
    SET(Clang_DIR_RE K:/llvm-build/Release/lib/cmake/clang)

    MESSAGE(STATUS Build Type: ${CMAKE_BUILD_TYPE})

    set(LLVM_PATH ${LLVM_PATH_DEV})
    set(LLVM_DIR ${LLVM_DIR_DEV})
    set(Clang_DIR ${Clang_DIR_DEV})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LLVM_PATH ${LLVM_PATH_DEB})
        set(LLVM_DIR ${LLVM_DIR_DEB})
        set(Clang_DIR ${Clang_DIR_DEB})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(LLVM_PATH ${LLVM_PATH_REL})
        set(LLVM_DIR ${LLVM_DIR_REL})
        set(Clang_DIR ${Clang_DIR_REL})
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(LLVM_PATH ${LLVM_PATH_DEV})
        set(LLVM_DIR ${LLVM_DIR_DEV})
        set(Clang_DIR ${Clang_DIR_DEV})
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${LLVM_DIR} ${Clang_DIR})
    find_package(LLVM)

    message(STATUS ${ZLIB_LIBRARY})
    message(STATUS ${LLVM_PATH})
    message(STATUS ${LLVM_DIR})
    message(STATUS ${Clang_DIR})

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
            Analysis ExecutionEngine InstCombine Object
            OrcJIT RuntimeDyld ScalarOpts native

            # MC
            target
        )
        MESSAGE(STATUS "${LLVM_LIRARIES}")
    ELSE()
        SET(WITH_LLVM 0)
        SET(LLVM_LIRARIES)
        MESSAGE(STATUS "LLVM not found!")
    ENDIF()
ENDIF()