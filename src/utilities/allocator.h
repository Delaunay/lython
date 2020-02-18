#ifndef LYTHON_UTILITIES_ALLOCATOR_HEADER
#define LYTHON_UTILITIES_ALLOCATOR_HEADER

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

namespace lython {

inline
int& _get_id(){
    static int i = 0;
    return i;
}

inline
std::vector<std::pair<int, int>> & stats(){
    static std::vector<std::pair<int, int>> stat;
    return stat;
}


inline
std::unordered_map<int, std::string> & typenames(){
    static std::unordered_map<int, std::string> stat;
    return stat;
}


inline
int _new_id(){
    auto r = _get_id();
    _get_id() += 1;
    stats().emplace_back(0, 0);
    return r;
}


template<typename T>
int type_id(){
    static int _id = _new_id();
    return _id;
}

template<typename T>
const char* _insert_typename(const char* str){
    typenames()[type_id<T>()] = str;
    return str;
}

template<typename T>
const char* type_name(){
    using C = typename T::nothing;
    static const char* name = _insert_typename<T>(
        std::string("<undefined(id="+ std::to_string(type_id<T>()) +")>").c_str());
    return name;
};

template<typename T>
int& allocated_count(){
    return stats()[type_id<T>()].first;
}

template<typename T>
int& deallocated_count(){
    return stats()[type_id<T>()].second;
}

void show_alloc_stats();


namespace device{

template<typename Device>
class DeviceAllocatorTrait{
    void* malloc(std::size_t n){
        return reinterpret_cast<Device*>(this)->malloc(n);
    }

    bool  free(void* ptr, std::size_t n){
        return reinterpret_cast<Device*>(this)->free(ptr, n);
    }
};

#ifdef __CUDACC__
struct CUDA: public DeviceAllocatorTrait<CUDA> {
    void* malloc(std::size_t n);
    bool free(void* ptr, std::size_t n);
};
#endif

struct CPU: public DeviceAllocatorTrait<CPU>  {
    void* malloc(std::size_t n);
    bool free(void* ptr, std::size_t n);
};

} // namespace device

template<typename T, typename Device>
class Allocator{
public:
    using size_type         = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T *;
    using const_pointer     = const T *;
    using reference         = T &;
    using const_reference   = const T &;
    using value_type        = T;

    template<typename _Tp1>
    struct rebind
    {
        using other = Allocator<_Tp1, Device>;
    };

    bool operator==(Allocator const&){
        return true;
    }

    bool operator!=(Allocator const& alloc){
        return !(*this == alloc);
    }

    void deallocate(pointer p, std::size_t n){
        deallocated_count<T>() += n * sizeof(T);
        allocator.free(static_cast<void*>(p), n * sizeof(T));
        return;
    }

    T* allocate(std::size_t n, const void *hint = nullptr){
        allocated_count<T>() += n * sizeof(T);
        return static_cast<T*>(allocator.malloc(n * sizeof(T)));
    }

    Allocator() noexcept
    {}

    Allocator(const Allocator &a) noexcept
    {}

    template <class U>
    Allocator(const Allocator<U, Device> &a) noexcept
    {}

    ~Allocator() noexcept = default;

private:
    Device allocator;
};

} // namespace lython

#endif
