#
#   If people don't build test don't add gtest
#
IF(BUILD_TESTING)
    # compile gtest
    ADD_SUBDIRECTORY(gtest)

    # does this work ?
    SET(${BUILD_GMOCK} ON)
    SET(${BUILD_GTEST} ON)

ENDIF(BUILD_TESTING)
