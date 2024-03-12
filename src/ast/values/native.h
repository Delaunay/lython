#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

#include "dtypes.h"
#include "utilities/object.h"
#include "utilities/allocator.h"

#include <type_traits>

namespace lython {

inline 
std::string _type_error(int expected, int got) {
    std::string expected_t = meta::type_name(expected);
    std::string got_t = meta::type_name(got);
    return "Wrong argument type, expected: `" + expected_t + "` got `" + got_t + "`";
}

template<typename ExpectedT>
std::string _type_error(int got) {
    std::string expected_t = meta::type_name<ExpectedT>();
    std::string got_t = meta::type_name(got);
    return "Wrong argument type, expected: `" + expected_t + "` got `" + got_t + "`";
}

class WrongTypeException: public std::exception {
public:
    WrongTypeException(std::string const& msg):
        message(msg)
    {}

    WrongTypeException(int expected, int got):
        message(_type_error(expected, got))
    {}

    const char* what() const noexcept override  {
        return message.c_str();
    }

private:
    std::string message;
};



// TODO: allow native object to register their own methods
// Is the reflection system enough for this to work on methods
struct NativeObject: public GCObject {

public:
    NativeObject(int type):
        _type(type)
    {}

    virtual bool is_native() const { return true; }

    virtual int8* _memory() = 0;

    int type() const { return _type;}

    virtual bool is_valid() const = 0;

    virtual bool is_pointer() const = 0;

    bool is_value() const { return !is_pointer(); }
    
    template<typename MemberT, typename MemberLookup>
    MemberT* member(MemberLookup name) {
        if (!is_valid()) {
            lyassert(0, "Underlying object is null");
            return nullptr;
        }
        meta::Member const& member = _get_member(name);

        if (member.type != meta::type_id<MemberT>()) {
            lyassert(0, "Member type should match");
            return nullptr;
        }

        int8* address = (_memory() + member.offset);

        MemberT* member_address = reinterpret_cast<MemberT*>(address);
        return member_address;
    }

    struct ConstantValue cmember(int member_id);

    template<typename ObjectT>
    bool is_type() const {
        return is_type(meta::type_id<ObjectT>());
    }

    bool is_type(int type_) const {
        return _type == type_;
    }

    template<typename ObjectT>
    ObjectT* as() {
        if (is_type<ObjectT>()) {
            return reinterpret_cast<ObjectT*>(_memory());
        }
        throw WrongTypeException(meta::type_id<ObjectT>(), _type);
    }

    template<typename ObjectT>
    ObjectT* as_fast() noexcept {
        return reinterpret_cast<ObjectT*>(_memory());
    }

    #if LYTHON_V0_BADCALL
    using ArgList = std::vector<NativeObject*>;

    template<int Index, typename Tuple>
    void convert_args(Tuple& result, ArgList::iterator begin, ArgList::iterator end) {
    }

    template<int Index, typename Tuple, typename T, typename... Args>
    void convert_args(Tuple& result, ArgList::iterator begin, ArgList::iterator end) {
        // We are making a copy here
        // for this to execute we need to know the argumentss
        std::get<Index>(result) = *((*begin)->as<T>());
        convert_args<Index + 1, Tuple, Args...>(result, ++begin, end);
    }

    // Safe-ish version convert the object to the right type
    template<typename ReturnT, typename... Args>
    ReturnT invoke(void* obj, std::string const& name, std::vector<NativeObject*> const& args) 
    {
        // invoke("sum", NativeObject(), NativeObject())
        // Arguments = Tuple<int*, float*>
        //
        //
        using Arguments = Tuple<void*, Args...>;
        using FunctionT = ReturnT(*)(void*, Args...);

        Arguments arguments;
        std::get<0>(arguments) = obj;
        convert_args<1, Arguments, Args...>(arguments, args.begin(), args.end());

        meta::Member const& member = _get_member(name);

        FunctionT fun = reinterpret_cast<FunctionT>(member.method);
        return std::apply(fun, arguments);
    }   
    #endif

    // Call a function but we do not know the types of the arguments
    NativeObject* invoke(void* obj, std::string const& name, std::vector<NativeObject*> const& args, GCObject* owner=nullptr)  {
        using FunctionT =  NativeObject*(*)(GCObject*, void*, meta::VoidFunction, std::vector<NativeObject*> const& args);

        // Simply call the generated wrapper
        meta::Member const& member = _get_member(name);

        if (owner == nullptr) {
            owner = this;
        }

        auto arguments = std::make_tuple(owner, obj, member.native, args);

        FunctionT fun = reinterpret_cast<FunctionT>(member.method);

        return std::apply(fun, arguments);        
    }

protected:
    meta::Member const& _get_member(int i) const {
        return meta::member(_type, i);
    }

    meta::Member const& _get_member(std::string const& i) const {
        return meta::member(_type, i);
    }

private:
    const int _type;
};

// Not sure this makes much sense
// it needs to be stack allocated anyway
template<typename T>
struct NativeValue: public NativeObject {
public:
    NativeValue():
        NativeObject(meta::type_id<T>())
    {
        meta::register_members<T>();
    }

    template<typename... Args>
    NativeValue(Args... args):
        NativeObject(meta::type_id<T>()), object(args...)
    {}

    int8* _memory() override  {
        return reinterpret_cast<int8*>(&object);
    }

    bool is_pointer() const override { return false; }

    bool is_valid() const override { return true; }

protected:
    T object;
};

template<typename T>
struct NativePointer: public NativeObject {
    NativePointer(T* o = nullptr):
        NativeObject(meta::type_id<T>()), object(o)
    {}

    NativePointer(NativeValue<T>& value):
        NativeObject(meta::type_id<T>()), object(value.template as<T>())
    {}

    /*
    static NativePointer* make() {
        // Problem here is how to know when to free 2 or one
        // and it is not part of the GC system
        void* mem = malloc(sizeof(NativePointer<T>) * sizeof(T));
        void* obj = mem + sizeof(NativePointer<T>);
        return new (mem) NativePointer(obj);
    }
    */

    void set_pointer(T* o) {
        object = o;
    }

    void clear() {
        object = nullptr;
    }

    bool is_valid() const override { return object != nullptr; }

    bool is_pointer() const override { return true; }

    int8* _memory() override  {
        return reinterpret_cast<int8*>(object);
    }

    void free() {
        if (object) {
            delete object;
        }
    }

protected:
    T* object = nullptr;
};




}  // namespace lython

#endif