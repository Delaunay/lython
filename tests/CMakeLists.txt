#
#   issue: what if *_test need a specific library ?
#

# tests need source header
INCLUDE_DIRECTORIES(../src)

SET(TEST_HEADERS
    samples.h
    lexer_test.h
    parser_test.h
)

INCLUDE(CTest)

# find_package(Catch2)
# INCLUDE(catch)
INCLUDE_DIRECTORIES(../dependencies/catch2/single_include)

SET(${CXX_STANDARD_REQUIRED} ON)

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME) # LIBRARIES
    ADD_EXECUTABLE(${NAME}_test ${NAME}_test.cpp ${TEST_HEADERS})
    TARGET_LINK_LIBRARIES(${NAME}_test Catch2::Catch2 liblython liblogging)
    SET_PROPERTY(TARGET ${NAME}_test PROPERTY CXX_STANDARD 17)
ENDMACRO(TEST_MACRO)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)
TEST_MACRO(main)
