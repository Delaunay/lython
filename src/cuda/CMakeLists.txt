# required for C++11 features
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# export PATH="$PATH:/usr/local/cuda-10.2/bin"
PROJECT(cuda LANGUAGES CXX CUDA)

ADD_EXECUTABLE(main main.cu)
TARGET_LINK_LIBRARIES(main pthread fmt::fmt liblogging liblython)
