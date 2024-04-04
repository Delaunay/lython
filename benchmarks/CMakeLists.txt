

INCLUDE_DIRECTORIES(../src)
INCLUDE_DIRECTORIES(../dependencies/xxHash)

# ADD_COMPILE_OPTIONS(-mavx512f)
IF(UNIX)
    ADD_COMPILE_OPTIONS(-mavx2)
ENDIF(UNIX)

ADD_EXECUTABLE(bench_hash bench_hash.cpp ${TEST_HEADERS})
TARGET_LINK_LIBRARIES(bench_hash Catch2::Catch2 liblython liblogging liblythontest)
