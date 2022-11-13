#ifndef LYTHON_UTILITIES_ALLOCATOR_HEADER
#define LYTHON_UTILITIES_ALLOCATOR_HEADER

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "logging/logging.h"

namespace lython {

void show_alloc_stats();

namespace meta {

// NOTE: All those should not depend on each other during deinit time
// https://isocpp.org/wiki/faq/ctors#construct-on-first-use-v2
struct Stat {
    int allocated     = 0;
    int deallocated   = 0;
    int bytes         = 0;
    int size_alloc    = 0;
    int size_free     = 0;
    int startup_count = 0;
};

bool& is_type_registry_available();

struct TypeRegistry {
    std::vector<Stat>                    stat;
    bool                                 print_stats = false;
    std::unordered_map<int, std::string> id_to_name;
    int                                  type_counter = 0;

    static TypeRegistry& instance() {
        static TypeRegistry obj;
        return obj;
    }

    TypeRegistry() { is_type_registry_available() = true; }

    ~TypeRegistry() {
        if (print_stats) {
            show_alloc_stats();
        }

        is_type_registry_available() = false;
    }
};

inline std::vector<Stat>& stats() { return TypeRegistry::instance().stat; }

inline int& _get_id() { return TypeRegistry::instance().type_counter; }

inline int _new_id() {
    auto r = _get_id();
    _get_id() += 1;
    stats().push_back(Stat());
    return r;
}

inline std::unordered_map<int, std::string>& typenames() {
    return TypeRegistry::instance().id_to_name;
}

// Generate a unique ID for a given type
template <typename T>
int type_id() {
    static int _id = _new_id();
    return _id;
}

template <typename T>
int _register_type_once(const char* str) {
    if (!is_type_registry_available())
        return 0;

    auto tid    = type_id<T>();
    auto result = typenames().find(tid);

    if (result == typenames().end()) {
        typenames().insert({type_id<T>(), str});
    }
    return tid;
}

// Insert a type name override
template <typename T>
const char* register_type(const char* str) {
    static int _ = _register_type_once<T>(str);
    return str;
}

// Return the type name of a function
// You can specialize it to override
template <typename T>
const char* type_name() {
    auto result = typenames().find(type_id<T>());

    if (result == typenames().end()) {
        const char* name = typeid(T).name();
        register_type<T>(name);
        return "<none>";
    }

    return (result->second).c_str();
};

inline const char* type_name(int class_id) {
    std::string const& name = typenames()[class_id];
    return name.c_str();
};

template <typename T>
Stat& get_stat() {
    return stats()[type_id<T>()];
}

// When type info is not available at compile time
// often when deleting a derived class
inline Stat& get_stat(int class_id) { return stats()[class_id]; }

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
        return;
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

template <typename V>
using UniquePtr = std::unique_ptr<V>;

template <typename _Tp, typename... _Args>
inline UniquePtr<_Tp> make_unique(_Args&&... __args) {
    auto ptr = Allocator<_Tp, device::CPU>().allocate(1);
    return UniquePtr<_Tp>(new (ptr) _Tp(std::forward<_Args>(__args)...));
}

}  // namespace lython

#endif
