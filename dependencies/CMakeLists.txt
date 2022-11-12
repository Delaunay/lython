#
# If people don't build test don't add gtest
#
IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(catch2)
    LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/dependencies/catch2/CMake")
ENDIF(BUILD_TESTING)

ADD_SUBDIRECTORY(spdlog)
ADD_SUBDIRECTORY(pybind11)

IF(NOT ${WITH_VALGRIND})
    ADD_SUBDIRECTORY(mimalloc EXCLUDE_FROM_ALL)
ENDIF()

ADD_SUBDIRECTORY(argparse EXCLUDE_FROM_ALL)

# this has dependencies on libtommath
# ADD_SUBDIRECTORY(libtomcrypt)
