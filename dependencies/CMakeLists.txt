#
#   If people don't build test don't add gtest
#
IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(catch2)
ENDIF(BUILD_TESTING)
