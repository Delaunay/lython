#
#   issue: what if *_test need a specific library ?
#

# tests need source header
INCLUDE_DIRECTORIES(../src)
INCLUDE(CTest)

SET(TEST_HEADERS
    samples.h
    lexer_test.cpp
    parser_test.cpp
    interpreter_test.cpp
    pool_test.cpp
    utilities_test.cpp
)

# find_package(Catch2)
# INCLUDE(catch)
INCLUDE_DIRECTORIES(../dependencies/catch2/single_include)

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME) # LIBRARIES
    ADD_EXECUTABLE(${NAME}_test ${NAME}_test.cpp ${TEST_HEADERS})
    TARGET_LINK_LIBRARIES(${NAME}_test spdlog::spdlog Catch2::Catch2 liblython liblogging)

    ADD_DEPENDENCIES(${NAME}_test liblython liblogging)
	
	IF(UNIX)
		ADD_DEPENDENCIES(coverage ${NAME}_test)
	ENDIF(UNIX)
	
    ADD_TEST(
        NAME ${NAME}
        WORKING_DIRECTORY .
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_test)
ENDMACRO(TEST_MACRO)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)
TEST_MACRO(main)



