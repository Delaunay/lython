#
#   issue: what if *_test need a specific library ?
#

# tests need source header
INCLUDE_DIRECTORIES(../src)

# I am not sure we need this
INCLUDE_DIRECTORIES(${gtest_dir})
INCLUDE_DIRECTORIES(${gtest_dir}/include/)

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME) # LIBRARIES
    ADD_EXECUTABLE(${NAME}_test ${NAME}_test.cpp)
    TARGET_LINK_LIBRARIES(${NAME}_test liblython gtest -pthread)
    # TARGET_LINK_LIBRARIES(${NAME}_test ${LIBRARIES} gtest -pthread)

    ADD_TEST(NAME ${NAME}_test
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_test)
    
    # gtest need to be compiled first
    ADD_DEPENDENCIES(${NAME}_test gtest)
ENDMACRO(TEST_MACRO)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)
TEST_MACRO(lexer)

