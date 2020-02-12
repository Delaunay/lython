# required for C++11 features
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# PROJECT(cuda CXX CUDA)

ADD_EXECUTABLE(main main.cpp)
TARGET_LINK_LIBRARIES(main pthread fmt::fmt liblogging liblython)
