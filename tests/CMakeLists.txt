#
#   issue: what if *_test need a specific library ?
#

# tests need source header
INCLUDE_DIRECTORIES(../src)
INCLUDE(CTest)

SET(TEST_HEADERS

)

# find_package(Catch2)
# INCLUDE(catch)
INCLUDE_DIRECTORIES(../dependencies/catch2/single_include)

ADD_LIBRARY(liblythontest cases_errors.cpp cases.cpp samples.cpp)
TARGET_LINK_LIBRARIES(liblythontest spdlog::spdlog Catch2::Catch2 liblython liblogging)

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME) # LIBRARIES
  ADD_EXECUTABLE(${NAME}_test main.cpp ${NAME}_test.cpp  ${TEST_HEADERS})
  TARGET_LINK_LIBRARIES(${NAME}_test spdlog::spdlog Catch2::Catch2 liblython liblogging liblythontest)
  ADD_DEPENDENCIES(${NAME}_test liblython liblogging liblythontest)

	IF(UNIX)
    ADD_DEPENDENCIES(coverage ${NAME}_test)
    ADD_DEPENDENCIES(coverage ${NAME}_test)
    ADD_DEPENDENCIES(coverage ${NAME}_test)
	ENDIF(UNIX)
	 
  ADD_TEST(
      NAME ${NAME}
      WORKING_DIRECTORY .
      COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_test)
ENDMACRO(TEST_MACRO)

# ADD_TEST(
#    NAME lython
#    WORKING_DIRECTORY .
#    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lython)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)
TEST_MACRO(logging)
TEST_MACRO(sema)
TEST_MACRO(lexer)
TEST_MACRO(parser)
TEST_MACRO(equality)
TEST_MACRO(pool)
TEST_MACRO(utilities)
TEST_MACRO(attribute)



