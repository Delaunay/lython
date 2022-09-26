#include "gemm.h"
#include "gemm_cuda.h"
#include "logging/logging.h"

int main() {
    int size = 256;

    info("{}", size);

    // test_gemm_parallel(size);

    info("GPU");

    test_gemm_cuda(2048);

    return 0;
}
