#ifndef LYTHON_METADATA_H
#define LYTHON_METADATA_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <ostream>

namespace lython {

struct Value;

using ValueDeleter = void(*)(Value&);
using ValueCopier  = Value(*)(Value const&);
using ValuePrinter = void(*)(std::ostream&, Value const&);
using ValueHash    = std::size_t(*)(Value const&);
using ValueRef      = Value(*)(Value&);

struct _None {
    bool operator== (_None const& val) const {
        return true;
    }
};

struct _Invalid {
   bool operator== (_Invalid const& val) const {
        return true;
    }  
};

namespace meta {

#define KIWI_VALUE_TYPES(X)     \
    X(bool, i1)      \
    X(uint64, u64)   \
    X(int64, i64)    \
    X(uint32, u32)   \
    X(int32, i32)    \
    X(uint16, u16)   \
    X(int16, i16)    \
    X(uint8, u8)     \
    X(int8, i8)      \
    X(float32, f32)  \
    X(float64, f64)  \
    X(Function, fun) \
    X(_None, none)   \
    X(_Invalid, invalid)

enum class ValueTypes
{
#define ENUM(type, name) name,
    KIWI_VALUE_TYPES(ENUM)
#undef ENUM
    Max
};


bool& is_type_registry_available();

// NOTE: All those should not depend on each other during deinit time
// https://isocpp.org/wiki/faq/ctors#construct-on-first-use-v2
struct AllocationStat {
    int allocated     = 0;
    int deallocated   = 0;
    int bytes         = 0;
    int size_alloc    = 0;
    int size_free     = 0;
    int startup_count = 0;
};


// Data and Function cannot use the same void
using VoidFunction = void(*)();
using VoidObject = void*;


struct Member {
    Member(std::string const& n, int t, int o, int s, VoidFunction na = nullptr, VoidFunction m=nullptr) :
        name(n), type(t), offset(o), size(s), native(na), method(m)
    {}

    std::string name;
    int   type = -1; 
    int   offset = -1;
    int   size = -1;
    VoidFunction native = nullptr;
    VoidFunction method = nullptr;
};

struct ClassMetadata {
    std::string         name;
    std::vector<Member> members;
    int                 offset  = +0;
    int                 type_id = -1;
    AllocationStat      stat;

    ValueDeleter deleter = nullptr;
    ValueCopier  copier  = nullptr;
    ValuePrinter printer = nullptr;
    ValueHash    hasher  = nullptr;
    ValueRef     ref     = nullptr;
};

struct TypeRegistry {
    bool                                   print_stats = false;
    std::unordered_map<int, ClassMetadata> id_to_meta;
    int                                    type_counter = int(ValueTypes::Max);

    static TypeRegistry& instance();

    TypeRegistry();

    ~TypeRegistry();

    void dump(std::ostream& out);
};

inline int& _get_id() { return TypeRegistry::instance().type_counter; }

inline int _new_type() {
    auto r = _get_id();
    _get_id() += 1;
    TypeRegistry::instance().id_to_meta[r].type_id = r;
    return r;
}


template <typename T>
struct _type_id {
    static int id() {
        static int _id = _new_type();
        return _id;
    }
};

template <typename T>
int type_id() {
    return _type_id<T>::id();
}

// Generate a unique ID for a given type
int _register_type_once(int tid, const char* str);

template <typename T>
int _register_type_once(const char* str) {
    return _register_type_once(type_id<T>(), str);
}

// Insert a type name override
template <typename T>
void override_typename(const char* str) {
    auto tid         = type_id<T>();
    TypeRegistry::instance().id_to_meta[tid].name = str;
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
    auto& db = TypeRegistry::instance().id_to_meta;

    auto result = db.find(type_id<T>());

    if (result == db.end() || result->second.name == "") {
        const char* name = typeid(T).name();
        return register_type<T>(name);
    }

    return (result->second).name.c_str();
};

inline const char* type_name(int class_id) {
    std::string const& name = TypeRegistry::instance().id_to_meta[class_id].name;
    return name.c_str();
};

template<typename ObjectT>
std::int8_t* member_address(ObjectT* value, Member const& member) {
    std::int8_t* mem = reinterpret_cast<std::int8_t*>((void*)value);
    std::int8_t* address = (mem + member.offset); 
    return address;
}

Member const& nomember();
Member const& member(int _typeid, std::string const& name);
Member const& member(int _typeid, int id);

std::tuple<int, int> member_id(int _typeid, std::string const& name);

// Low Level API to retrieve the classmeta
ClassMetadata& classmeta(int _typeid);

template<typename T>
struct ReflectionTrait {
    static int register_members() {
        return 0;
    }
};

template<typename T>
void register_members() {
    static int _ = ReflectionTrait<T>::register_members();
}

// High level API to retrive the classmeta
// register the members if not done already
template<typename T>
ClassMetadata& classmeta() {
    register_members<T>();
    return classmeta(type_id<T>());
}

template<typename T, typename U>
void new_member(std::string const& name) {
    ClassMetadata& registry = classmeta(type_id<T>());
    int size = sizeof(U);
    registry.members.emplace_back(name, type_id<U>(), registry.offset, size);
    registry.offset += size;
}


template<typename T>
Member const& member(std::string const& name) {
    return member(type_id<T>(), name);
}

template<typename T>
Member const& member(int member_id) {
    return member(type_id<T>(), member_id);
}


void print(std::ostream& ss, int typeid_, std::int8_t const* data);

template<typename T>
void print(std::ostream& ss, T const& value) {
    ClassMetadata const& registry = classmeta<T>();

    if (registry.printer) {
        registry.printer(ss, reinterpret_cast<std::int8_t const*>(&value));
        return;
    }   

    ss << type_name<T>() << "(";
    bool is_first = true;
    for(Member const& mem: registry.members) {
        if (mem.method != nullptr) {
            continue;
        }

        if (!is_first) {
            ss << ", ";
        }
        is_first = false;

        ss << mem.name << "=";

        print(ss, mem.type, member_address(&value, mem));
    }
    ss << ")";
};

}

void metadata_init_names();

void track_static();

void register_globals();


}  // namespace lython

#endif
