#ifndef LYTHON_CUDA_IDE_H
#define LYTHON_CUDA_IDE_H

#ifndef __CUDACC__
#    include <cuda_runtime.h>

struct __block {
    int x, y, z;
};

extern __block __ignore;

#    define blockIdx  __ignore
#    define blockDim  __ignore
#    define threadIdx __ignore
#endif

#endif
