#ifndef LYTHON_UTILITIES_ALLOCATOR_HEADER
#define LYTHON_UTILITIES_ALLOCATOR_HEADER

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef __clang__
#define KWMETA(...) __attribute__((annotate(#__VA_ARGS__))) 
#else
#define KWMETA(...)
#endif


#include "utilities/metadata_1.h"
#include "logging/logging.h"

namespace lython {

void show_alloc_stats();

namespace meta {

// When type info is not available at compile time
// often when deleting a derived class
AllocationStat& get_stat(int class_id);

template <typename T>
AllocationStat& get_stat() {
    return get_stat(type_id<T>());
}
 
}  // namespace meta

inline void show_alloc_stats_on_destroy(bool enabled) {
    meta::TypeRegistry::instance().print_stats = enabled;
}

namespace device {

template <typename Device>
class DeviceAllocatorTrait {
    static void* malloc(std::size_t n) { return Device::malloc(n); }

    static bool free(void* ptr, std::size_t n) { return Device::free(ptr, n); }
};

#ifdef __CUDACC__
struct CUDA: public DeviceAllocatorTrait<CUDA> {
    void* malloc(std::size_t n);
    bool  free(void* ptr, std::size_t n);
};
#endif

struct CPU: public DeviceAllocatorTrait<CPU> {
    static void* malloc(std::size_t n);

    static bool free(void* ptr, std::size_t n);
};

}  // namespace device

inline void manual_free(int class_id, std::size_t n) {
    meta::get_stat(class_id).deallocated += 1;
    meta::get_stat(class_id).size_free += int(n);
}

template <typename T, typename Device>
class Allocator {
    public:
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using value_type      = T;

    template <typename _Tp1>
    struct rebind {
        using other = Allocator<_Tp1, Device>;
    };

    bool operator==(Allocator const&) const { return true; }

    bool operator!=(Allocator const& alloc) const { return !(*this == alloc); }

    // template <typename... Args>
    // static void construct(T *value, Args &&...args) {
    //     new ((void *)value) T(std::forward<Args>(args)...);
    // }

    static void deallocate(pointer p, std::size_t n) {
        manual_free(meta::type_id<T>(), n);
        Device::free(static_cast<void*>(p), n * sizeof(T));
    }

    static T* allocate(std::size_t n, const void* = nullptr) {
        meta::register_type<T>(typeid(T).name());
        meta::get_stat<T>().allocated += 1;
        meta::get_stat<T>().size_alloc += int(n);
        meta::get_stat<T>().bytes = int(sizeof(T));
        return static_cast<T*>(Device::malloc(n * sizeof(T)));
    }

    Allocator() noexcept {}

    Allocator(const Allocator& a) noexcept {}

    template <class U>
    Allocator(const Allocator<U, Device>& a) noexcept {}

    ~Allocator() noexcept = default;
};

template <typename V>
using SharedPtr = std::shared_ptr<V>;

template <typename _Tp, typename... _Args>
inline SharedPtr<_Tp> make_shared(_Args&&... __args) {
    typedef typename std::remove_cv<_Tp>::type _Tp_nc;
    return std::allocate_shared<_Tp>(Allocator<_Tp_nc, device::CPU>(),
                                     std::forward<_Args>(__args)...);
}

template <typename V, typename ...Args>
using UniquePtr = std::unique_ptr<V, Args...>;

template <typename _Tp, typename... _Args>
inline UniquePtr<_Tp> make_unique(_Args&&... __args) {
    auto ptr = Allocator<_Tp, device::CPU>().allocate(1);
    return UniquePtr<_Tp>(new (ptr) _Tp(std::forward<_Args>(__args)...));
}

}

#endif
