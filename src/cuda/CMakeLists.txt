# required for C++11 features
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# export PATH="$PATH:/usr/local/cuda-10.2/bin"
PROJECT(cuda LANGUAGES CXX CUDA)

ADD_EXECUTABLE(main main.cu)
TARGET_LINK_LIBRARIES(main pthread liblogging liblython)


ADD_EXECUTABLE(gemm
    ../utilities/allocator.cu
    ../math/main.cpp
    ../math/gemm.cpp
    ../math/gemm.h
    ../math/gemm_cuda.cu
    ../math/gemm_cuda.h)
TARGET_LINK_LIBRARIES(gemm liblython liblogging)
