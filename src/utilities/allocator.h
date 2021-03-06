#ifndef LYTHON_UTILITIES_ALLOCATOR_HEADER
#define LYTHON_UTILITIES_ALLOCATOR_HEADER

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

#include "logging/logging.h"

namespace lython {

namespace meta{

struct Stat{
    int allocated   = 0;
    int deallocated = 0;
    int bytes       = 0;
    int size_alloc  = 0;
    int size_free   = 0;
};

inline
std::vector<Stat> & stats(){
    static std::vector<Stat> stat;
    return stat;
}


inline
int& _get_id(){
    static int i = 0;
    return i;
}

inline
int _new_id(){
    auto r = _get_id();
    _get_id() += 1;
    stats().emplace_back();
    return r;
}


inline
std::unordered_map<int, std::string> & typenames(){
    static std::unordered_map<int, std::string> stat;
    return stat;
}

// Generate a unique ID for a given type
template<typename T>
int type_id(){
    static int _id = _new_id();
    return _id;
}

// Insert a type name override
template<typename T>
const char* register_type(const char* str){
    if (typenames()[type_id<T>()].size() <= 0){
        typenames()[type_id<T>()] = str;
    }
    return str;
}

// Return the type name of a function
// You can specialize it to override
template<typename T>
const char* type_name(){
    std::string const& name = typenames()[type_id<T>()];

    if (name.size() <= 0){
        const char* name = typeid(T).name();
        register_type<T>(name);
        return name;
    }

    return name.c_str();
};


template<typename T>
Stat& get_stat(){
    return stats()[type_id<T>()];
}

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
        meta::get_stat<T>().deallocated += 1;
        meta::get_stat<T>().size_free += n;
        allocator.free(static_cast<void*>(p), n * sizeof(T));
        return;
    }

    T* allocate(std::size_t n, const void * = nullptr){
        meta::register_type<T>(typeid(T).name());

        meta::get_stat<T>().allocated += 1;
        meta::get_stat<T>().size_alloc += n;
        meta::get_stat<T>().bytes = sizeof(T);
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


template<typename V>
using SharedPtr = std::shared_ptr<V>;

template<typename _Tp, typename... _Args>
inline SharedPtr<_Tp> make_shared(_Args&&... __args)
{
    typedef typename std::remove_cv<_Tp>::type _Tp_nc;
    return std::allocate_shared<_Tp>(
        Allocator<_Tp_nc, device::CPU>(), std::forward<_Args>(__args)...);
}

template<typename V>
using UniquePtr = std::unique_ptr<V>;

template<typename _Tp, typename... _Args>
inline UniquePtr<_Tp> make_unique(_Args&&... __args){
    auto ptr = Allocator<_Tp, device::CPU>().allocate(1);
    return UniquePtr<_Tp>(new (ptr) _Tp(std::forward<_Args>(__args)...));
}

} // namespace lython

#endif
