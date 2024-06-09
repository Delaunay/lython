#
#
#
IF(CMAKE_CXX_COMPILER MATCHES "/em.*$")

ENDIF()

#
#
#
IF(UNIX)

ENDIF()

#
#
#
IF(WIN32)

    SET(Python_ROOT_DIR "E:/Anaconda")
    FIND_PACKAGE(Python)
ENDIF()