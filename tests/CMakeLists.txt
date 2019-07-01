#
#   issue: what if *_test need a specific library ?
#

# tests need source header
INCLUDE_DIRECTORIES(../src)
include(GoogleTest)

SET(TEST_HEADERS
    lexer_test.h
    parser_test.h
)

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME) # LIBRARIES
    ADD_EXECUTABLE(${NAME}_test ${NAME}_test.cpp ${TEST_HEADERS})
    TARGET_LINK_LIBRARIES(${NAME}_test liblython liblogging gtest pthread)
ENDMACRO(TEST_MACRO)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)
TEST_MACRO(main)
