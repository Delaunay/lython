#include <cassert>
#include <future>

#include "logging/logging.h"
#include "utilities/pool.h"
#include "utilities/stopwatch.h"

lython::ThreadPool& pool() {
    static lython::ThreadPool pool;
    return pool;
}

struct Memory {
    float* ptr = nullptr;
};

struct Strides {
    int x     = 0;
    int y     = 0;
    int total = 0;

    int operator[](int i) {
        switch (i) {
        default:
        case 0: return x;
        case 1: return y;
        }
    }
};

struct Matrix {
    Matrix(int n, int m):
        // ColMajor (n, 1)
        // RowMajor (1, n)
        row(n), col(m), stride({n, 1, n * m}) {}

    float& operator()(int x, int y) { return mem.ptr[x * stride.x + y * stride.y]; }

    int row;
    int col;

    Strides stride;
    Memory  mem;
};

void gemm_single_friendly(Matrix* a, Matrix* b, Matrix* c) {
    // (n x m) . (m x p) => (n x p)
    assert(a->col == b->row, "");
    assert(c->row == a->row && c->col == b->col, "");

    for (int i = 0; i < c->row * c->col; ++i) {
        c->mem.ptr[i] = 0;
    }

    for (int n = 0; n < a->row; ++n) {
        // Col Major case iterates over the cols first
        for (int p = 0; p < c->col; ++p) {
            for (int m = 0; m < a->col; ++m) {
                (*c)(n, p) += (*a)(n, m) * (*b)(m, p);
            }
        }
    }
}

void gemm_single_bad(Matrix* a, Matrix* b, Matrix* c) {
    // (n x m) . (m x p) => (n x p)
    assert(a->col == b->row, "");
    assert(c->row == a->row && c->col == b->col, "");

    for (int i = 0; i < c->row * c->col; ++i) {
        c->mem.ptr[i] = 0;
    }

    // cache unfriendly version
    for (int p = 0; p < c->col; ++p) {
        for (int m = 0; m < a->col; ++m) {
            for (int n = 0; n < a->row; ++n) {
                (*c)(n, p) += (*a)(n, m) * (*b)(m, p);
            }
        }
    }
}

void gemm_parallel(Matrix a, Matrix b, Matrix c) {
    // (n x m) . (m x p) => (n x p)
    assert(a.col == b.row, "");
    assert(c.row == a.row && c.col == b.col, "");

    auto fun = [&](int n) {
        // Col Major case iterates over the cols first

        for (int p = 0; p < b.col; ++p) {
            float sum = 0;

            for (int m = 0; m < a.col; ++m) {
                // sum += (a)(n, m) * (b)(m, p);
                sum += a.mem.ptr[n * a.col + m] * b.mem.ptr[m * b.col + p];
            }

            c(n, p) = sum;
        }

        return true;
    };

    std::vector<std::future<bool>> tasks;
    tasks.reserve(std::size_t(a.row * b.col));

    for (int n = 0; n < a.row; ++n) {
        // tasks.emplace_back(std::async(std::launch::async, fun, n, p));
        tasks.emplace_back(pool().queue_task(fun, n));
    }

    for (auto& future: tasks) {
        future.wait();
    }
}

void gemm_parallel_block(Matrix a, Matrix b, Matrix c) {
    // (n x m) . (m x p) => (n x p)
    assert(a.col == b.row, "");
    assert(c.row == a.row && c.col == b.col, "");

    auto fun = [&](int n) {
        // Col Major case iterates over the cols first

        for (int p = 0; p < b.col; ++p) {
            float sum = 0;

            for (int m = 0; m < a.col; ++m) {
                // sum += (a)(n, m) * (b)(m, p);
                sum += a.mem.ptr[n * a.col + m] * b.mem.ptr[m * b.col + p];
            }

            c(n, p) = sum;
        }

        return true;
    };

    std::vector<std::future<bool>> tasks;
    tasks.reserve(std::size_t(a.row * b.col));

    for (int n = 0; n < a.row; ++n) {
        // tasks.emplace_back(std::async(std::launch::async, fun, n, p));
        tasks.emplace_back(pool().queue_task(fun, n));
    }

    for (auto& future: tasks) {
        future.wait();
    }
}

// [I] [16-02-2020 16:04:07.897] [8243] src/math/gemm.cpp:159 test_gemm_parallel - Total: 1467.5
std::vector<float> test_gemm_parallel(int size) {
    info("Vector size {} => {}", size, size * size);

    std::vector<float> a_ptr(std::size_t(size * size), 0);
    std::vector<float> b_ptr(std::size_t(size * size), 0);
    std::vector<float> c_ptr(std::size_t(size * size), 0);

    for (std::size_t i = 0; i < size * size; ++i) {
        a_ptr[i] = 1;
        b_ptr[i] = 1;
        c_ptr[i] = 0;
    }

    Matrix A(size, size);
    A.mem.ptr = &a_ptr[0];
    Matrix B(size, size);
    B.mem.ptr = &b_ptr[0];
    Matrix C(size, size);
    C.mem.ptr = &c_ptr[0];

    float average = 0;

    for (int j = 0; j < 10; ++j) {
        lython::StopWatch<> chrono;

        for (int i = 0; i < 10; ++i) {
            gemm_parallel(A, B, C);
        }

        average += float(chrono.stop());
    }

    info("Total: {}", average / 10);
    return c_ptr;
}
