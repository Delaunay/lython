#include "utilities/stopwatch.h"
#include "logging/logging.h"

#include "cuda_ide.h"

struct Event{
    Event(){
        cudaEventCreate(&event);
    }

    ~Event(){
        cudaEventDestroy(event);
    }

    operator cudaEvent_t& (){
        return event;
    }

    static float time(cudaEvent_t& start, cudaEvent_t& end){
        float f = 0;
        cudaEventElapsedTime(&f, start, end);
        return f;
    }

    void record(){
        cudaEventRecord(event, nullptr);
    }

    void wait(){
        cudaEventSynchronize(event);
    }

    cudaEvent_t event;
};

// row-major order
struct MatrixView{
    int width;
    int height;
    int stride;
    float* elements;

    __host__ __device__ float& operator() (int row, int col){
        return elements[row * stride + col];
    }

    __host__ __device__ MatrixView sub_block(int row, int col, int block_size){
        MatrixView sub;
        sub.width = block_size;
        sub.height = block_size;
        sub.stride = stride;
        sub.elements = &(*this)(row * block_size, col * block_size);
        return sub;
    }
};

struct Matrix{
    int width;
    int height;
    int stride;
    float* elements;
    bool cpu;

    Matrix(int rows, int cols, bool cpu=true):
        width(cols), height(cols), stride(width), cpu(cpu)
    {
        std::size_t size = rows * cols * sizeof(float);

        if (cpu){
            elements = static_cast<float*>(malloc(size));
        } else {
           cudaMalloc(&elements, size);
        }
    }

    ~Matrix(){
        if (cpu){
            free(elements);
        } else {
            cudaFree(elements);
        }
    }

    operator MatrixView(){
        MatrixView v;
        v.width = width;
        v.height = height;
        v.stride = stride;
        v.elements = elements;
        return v;
    }
};



// (n x m) . (m x p) => (n x p)
__global__ void matrix_mult_bad_kernel(float* u, float* v, float* ret, int size) {

    const int row = blockIdx.x * blockDim.x + threadIdx.x;
    const int col = blockIdx.y * blockDim.y + threadIdx.y;

    float sum = 0;

    if (row < size && col < size){
        for(int i = 0; i < size; ++i) {
            sum += u[i + row * size] + v[col + i * size];
        }

        ret[row * size + col] = sum;
    }
}



template<int tile> __global__
void matrix_mult_tile(MatrixView a, MatrixView b, MatrixView ret, int size) {
    // tile x tile threads running at the same time

    const int block_row = blockIdx.x;
    const int block_col = blockIdx.y;

    // each threads compute one (row, col) pair
    const int row = threadIdx.y;
    const int col = threadIdx.x;
    float sum = 0;

    for (int m = 0; m < (size / tile); ++m) {
        MatrixView sub_a = a.sub_block(block_row, m, tile);
        MatrixView sub_b = b.sub_block(m, block_col, tile);

        // Here each thread is going to fetch 2 values (a_ij b_jk)
        // and store them in the shared location
        // so everybody get access to the data without having to fetch
        // into global memory
        __shared__ float as[tile][tile];
        __shared__ float bs[tile][tile];

        as[row][col] = sub_a(row, col);
        bs[row][col] = sub_a(row, col);

        __syncthreads();

        // All data was fetched we can start computing;
        #pragma unroll
        for(int i = 0; i < tile; ++i){
            sum += as[row][i] * bs[i][col];
        }

        // make sure shared data is not needed anymore
        __syncthreads();
    }

    MatrixView sub_c = ret.sub_block(block_row, block_col, tile);
    sub_c(row, col) = sum;
}





void test_gemm_cuda(int size){
    // [I] [16-02-2020 15:38:07.653] [26786] src/math/gemm_cuda.cu:106 test_gemm_cuda_bad - Total: 2905.5

    Matrix da(size, size, false);
    Matrix db(size, size, false);
    Matrix dc(size, size, false);

    float average = 0;

    for(int j = 0; j < 5; ++j){
        lython::StopWatch<> chrono;
        Event start;
        Event stop;

        start.record();

        for(int i = 0; i < 2; ++i){
            // cudaConfigureCall(dim3 gridDim, dim3 blockDim, size_t sharedMem, cudaStream_t stream
            // cudaSetupArgumentconst (void* arg, size_t size, size_t offset)
            // cudaError_t cudaLaunch (const char *entry)
            // kernel<<<grid, threads, num_extern_shared_bytes, stream>>>(args)

            int tile = 32;

            dim3 dimBlock(tile, tile);
            dim3 dimGrid(size / dimBlock.x, size / dimBlock.y);

            matrix_mult_tile<32><<<dimGrid, dimBlock>>>(da, db, dc, size);
        }

        stop.record();
        stop.wait();
        average += float(chrono.stop());
    }

    kwinfo("Total: {}", average / 10);
}
