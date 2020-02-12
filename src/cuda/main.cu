// #include "utilities/pool.h"

#include "logging/logging.h"

#include <iostream>
// #include <cuda/cuda.h>


__global__ void add(float*a, float*b, float*c){
    c[blockIdx.x] = a[blockIdx.x] + b[blockIdx.x];
}


int main(){
    // force signal handler to be installed
    info("");

    int size = 10;
    float* da, *db, *dc;
    float* a, *b, *c;

    a = (float*) malloc(sizeof(float) * size);
    b = (float*) malloc(sizeof(float) * size);
    c = (float*) malloc(sizeof(float) * size);

    for(int i = 0; i < size; ++i){
        a[i] = i;
        b[i] = i;
        c[i] = 0;
    }

    cudaMalloc(&da, sizeof(float) * size);
    cudaMalloc(&db, sizeof(float) * size);
    cudaMalloc(&dc, sizeof(float) * size);
    // -------------------------------------

    cudaMemcpy(da, a, sizeof(float) * size, cudaMemcpyHostToDevice);
    cudaMemcpy(db, b, sizeof(float) * size, cudaMemcpyHostToDevice);

    add<<<size,1>>>(da, db, dc);

    cudaMemcpy(c, dc, sizeof(float) *size, cudaMemcpyDeviceToHost);

    for(int i = 0; i < size; ++i){
        std::cout << c[i] << std::endl;
    }

    cudaFree(da); free(a);
    cudaFree(db); free(b);
    cudaFree(dc); free(c);
    return 0;
}
