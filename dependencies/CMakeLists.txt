#
# If people don't build test don't add gtest
#
IF(BUILD_TESTING)
    # ADD_SUBDIRECTORY(catch2)
    LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/dependencies/catch2/CMake")
ENDIF(BUILD_TESTING)

IF(NOT BUILD_WEBASSEMBLY)
    # ADD_SUBDIRECTORY(pybind11)
ENDIF()

IF(NOT ${WITH_VALGRIND})
    # ADD_SUBDIRECTORY(mimalloc EXCLUDE_FROM_ALL)
ENDIF()

# ADD_SUBDIRECTORY(argparse EXCLUDE_FROM_ALL)
# ADD_SUBDIRECTORY(zlib EXCLUDE_FROM_ALL) 

# this has dependencies on libtommath
# ADD_SUBDIRECTORY(libtomcrypt)
# ADD_SUBDIRECTORY(llvm/libc)
# ADD_SUBDIRECTORY(llvm/libcxx)
# ADD_SUBDIRECTORY(llvm/libcxxabi)

#
#   You need to build SDL manually for it to work on windows
#
IF (NOT BUILD_WINDOWS)
    SET(BUILD_SHARED_LIBS off)
    # #ADD_SUBDIRECTORY(SDL)
    # ADD_SUBDIRECTORY(SDL_ttf)
ENDIF()

# ADD_SUBDIRECTORY(fmt)
# ADD_SUBDIRECTORY(backward-cpp)
# ADD_SUBDIRECTORY(zlib)
# ADD_SUBDIRECTORY(libuv)
# ADD_SUBDIRECTORY(flecs)
# ADD_SUBDIRECTORY(freetype)