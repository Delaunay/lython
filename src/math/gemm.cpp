#include <cassert>
#include <future>
#include <vector>

struct Memory{
    float* ptr = nullptr;
    int size = 0;
};

struct Strides{
    int x = 0;
    int y = 0;
    int total = 0;

    int operator[] (int i){
        switch (i) {
        default:
        case 0: return x;
        case 1: return y;
        }
    }
};


struct Matrix{
    Matrix(int n, int m):
        // ColMajor (n, 1)
        // RowMajor (1, n)
        row(n), col(m), stride({n, 1, n * m})
    {}

    float& operator()(int x, int y){
        return mem.ptr[x * stride.x + y * stride.y];
    }

    int row;
    int col;

    Strides stride;
    Memory mem;
};


void gemm_single_friendly(Matrix* a, Matrix* b, Matrix* c){
    // (n x m) . (m x p) => (n x p)
    assert(a->col == b->row);
    assert(c->row == a->row && c->col == b->col);

    for(int i = 0; i < c->row * c->col; ++i){
        c->mem.ptr[i] = 0;
    }

    for(int n = 0; n < a->row; ++n){
        // Col Major case iterates over the cols first
        for(int p = 0; p < c->col; ++p){
            for(int m = 0; m < a->col; ++m){
                (*c)(n, p) += (*a)(n, m) * (*b)(m, p);
            }
        }
    }
}

void gemm_single_bad(Matrix* a, Matrix* b, Matrix* c){
    // (n x m) . (m x p) => (n x p)
    assert(a->col == b->row);
    assert(c->row == a->row && c->col == b->col);

    for(int i = 0; i < c->row * c->col; ++i){
        c->mem.ptr[i] = 0;
    }

    // cache unfriendly version
    for(int p = 0; p < c->col; ++p){
        for(int m = 0; m < a->col; ++m){
            for(int n = 0; n < a->row; ++n){
                (*c)(n, p) += (*a)(n, m) * (*b)(m, p);
            }
        }
    }
}

void gemm_parallel(Matrix* a, Matrix* b, Matrix* c){
    // (n x m) . (m x p) => (n x p)
    assert(a->col == b->row);
    assert(c->row == a->row && c->col == b->col);

    for(int i = 0; i < c->row * c->col; ++i){
        c->mem.ptr[i] = 0;
    }

    std::atomic<int> done = 0;
    auto fun = [&](int n){
        // Col Major case iterates over the cols first
        for(int p = 0; p < c->col; ++p){
            for(int m = 0; m < a->col; ++m){
                (*c)(n, p) += (*a)(n, m) * (*b)(m, p);
            }
        }
        done += 1;
    };

    for(int n = 0; n < a->row; ++n){
        std::async(std::launch::async, fun, n);
    }

    while (done != a->row){
        std::this_thread::yield();
    }
}
