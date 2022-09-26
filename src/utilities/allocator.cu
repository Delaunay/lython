#if __has_include(<cuda_runtime_api.h>)
#    include <cuda_runtime_api.h>

#    include "allocator.h"

namespace lython {
namespace device {

// CUDA alloc is guaranteed to be 256 aligned
void* CUDA::malloc(std::size_t n) {
    void* ptr = nullptr;
    cudaMalloc(&ptr, sizeof(float) * n);
    return ptr;
}

bool CUDA::free(void* ptr, std::size_t) {
    cudaFree(ptr);
    return true;
}

}  // namespace device
}  // namespace lython
#endif
