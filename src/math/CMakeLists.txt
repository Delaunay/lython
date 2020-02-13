
ADD_EXECUTABLE(gemm main.cpp gemm.cpp gemm.h)
TARGET_LINK_LIBRARIES(gemm fmt::fmt liblython liblogging)
