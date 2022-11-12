


INCLUDE_DIRECTORIES(../src)
INCLUDE_DIRECTORIES(../dependencies/xxHash)
# ADD_COMPILE_OPTIONS(-mavx512f)
ADD_COMPILE_OPTIONS(-mavx2)

ADD_EXECUTABLE(bench_hash bench_hash.cpp ${TEST_HEADERS})
TARGET_LINK_LIBRARIES(bench_hash spdlog::spdlog Catch2::Catch2 liblython liblogging liblythontest)

