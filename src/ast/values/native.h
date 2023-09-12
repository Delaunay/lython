#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

#include "dtypes.h"
#include "utilities/object.h"
#include "utilities/allocator.h"

#include <type_traits>

namespace lython {



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
            assert(0, "Underlying object is null");
            return nullptr;
        }
        meta::Member const& member = _get_member(name);

        if (member.type != meta::type_id<MemberT>()) {
            assert(0, "Member type should match");
            return nullptr;
        }

        int8* address = (_memory() + member.offset);

        MemberT* member_address = reinterpret_cast<MemberT*>(address);
        return member_address;
    }

    template<typename ObjectT>
    ObjectT* as() {
        if (_type != meta::type_id<ObjectT>()) {
            assert(0, "object type should match");
            return nullptr;
        }

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
    NativeObject* invoke(void* obj, std::string const& name, std::vector<NativeObject*> const& args)  {
        using FunctionT =  NativeObject*(*)(void*, meta::VoidFunction, std::vector<NativeObject*> const& args);

        // Simply call the generated wrapper
        meta::Member const& member = _get_member(name);

        auto arguments = std::make_tuple(obj, member.native, args);

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
    {
        meta::register_members<T>();
    }

    NativePointer(NativeValue<T>& value):
        NativeObject(meta::type_id<T>()), object(value.as<T>())
    {}

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

protected:
    T* object = nullptr;
};




}  // namespace lython

#endif