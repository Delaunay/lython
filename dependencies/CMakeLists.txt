#
# If people don't build test don't add gtest
#
IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(catch2)
    LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/dependencies/catch2/CMake")
ENDIF(BUILD_TESTING)

ADD_SUBDIRECTORY(spdlog)
ADD_SUBDIRECTORY(pybind11)
ADD_SUBDIRECTORY(mimalloc)
ADD_SUBDIRECTORY(argparse)

# this has dependencies on libtommath
# ADD_SUBDIRECTORY(libtomcrypt)
