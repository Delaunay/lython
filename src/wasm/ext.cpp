

namespace emscripten {
    template <typename T>
    struct TypeID<T, std::enable_if_t<is_function<typename Canonicalized<T>::type>::value>>
    {
        static constexpr TYPEID get() { return TypeID<val>::get(); }
    };

    template <typename R, typename... Args>
    struct BindingType<std::function<R(Args...)>>
    {
        using ValBinding = BindingType<val>;
        using WireType = ValBinding::WireType;

        static WireType toWireType(const std::function<R(Args...)> &f) {
            // converter code
        }
        static std::function<R(Args...)> fromWireType(WireType value) {
            return [=](Args...args) -> R
            {
                val &&v = ValBinding::fromWireType(value);

                val r = v(args...);
                if constexpr (std::is_same_v<R, void>)
                {
                    // do nothing
                }
                else
                {
                    return r.template as<R>();
                }
            };
        }
    }

// define C function wrapper
typedef std::function<emscripten::val(emscripten::val)> cfunction_wrapper;
// the map to store the C function id and wrapper
std::map<std::string, cfunction_wrapper *> cfunction_map;

// helper function, used to find and call the target C function, and delete it?
emscripten::val cfunction_helper(std::string key, emscripten::val args)
{
    auto it = cfunction_map.find(key);
    if (it != cfunction_map.end())
    {
        auto func_ptr = it->second;
        emscripten::val r = (*func_ptr)(args);
        // delete it? should I?
        // cfunction_map.erase(it);
        // delete func_ptr;
        return r;
    }
    return emscripten::val::undefined();
}

EMSCRIPTEN_BINDINGS(😅)
{
    // register the helper function first
    function("cfunction_helper", &cfunction_helper);
    // register the function that would return a lambda value
    function("function_callback_function", &function_callback_function);
}

template <typename T>
struct StringTypeReplace {
    using type = std::string;
};

template <typename... Args>
static val NewFunctionVal(std::tuple<Args...> tuple)
{
    return std::apply(&val::new_<Args...>, std::tuple_cat(std::tuple(val::global("Function")), tuple));
}

template <class Tuple, std::size_t... I>
Tuple GetJsFunctionArgsDeclare(std::index_sequence<I...>)
{
    return Tuple(std::tuple_element_t<I, Tuple>(std::string("a") + std::to_string(I))...);
}

template <typename T, size_t I>
auto JsArgumentToCppObject(val args)
{
    return args[I].as<T>();
}

template <class Tuple, std::size_t... I>
Tuple ArgumentsToTuple_impl(val args, std::index_sequence<I...>)
{
    return Tuple{JsArgumentToCppObject<std::tuple_element_t<I, Tuple>, I>(args)...};
}

template <typename... Args>
std::tuple<std::decay_t<Args>...> ArgumentsToTuple(val args)
{
    return ArgumentsToTuple_impl<std::tuple<std::decay_t<Args>...>>(args, std::make_index_sequence<sizeof...(Args)>());
}

template <typename R, typename... Args>
struct BindingType<std::function<R(Args...)>>
{
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    static WireType toWireType(const std::function<R(Args...)> &f) {
        // wrap the std::function or lambda
        cfunction_wrapper wrap_func = [=](val args)
        {
            if (args.isArray()) {
                // convert argument array to std::tuple
                auto &&tuple = ArgumentsToTuple<Args...>(args);
                if constexpr (std::is_same_v<R, void>)
                {
                    std::apply(f, tuple);
                    return val::undefined();
                }
                else
                {
                    R &&r = std::apply(f, tuple);
                    return val(r);
                }
            }
            return val::undefined();
        };
        // use function pointer as function key, make it be unique
        auto func_ptr = new cfunction_wrapper(wrap_func);
        std::string key = std::to_string(reinterpret_cast<uintptr_t>(func_ptr));
        cfunction_map[key] = func_ptr;

        // convert Args... to string 'a0,a1,a2...' as js function arguments declaration
        using tuple_type = std::tuple<typename StringTypeReplace<Args>::type...>;
        auto tuple = GetJsFunctionArgsDeclare<tuple_type>(std::make_index_sequence<sizeof...(Args)>());        
        std::ostringstream oss;
        std::apply([&oss](const auto&... args) {
                ((oss << args << ","), ...);
            }, tuple);
        std::string arg_array_str = oss.str();
        if (!arg_array_str.empty()) {
            arg_array_str.erase(arg_array_str.size() - 1);
        }
        
        // create a function val that would call the helper function
        val &&func = NewFunctionVal(std::tuple_cat(tuple, std::tuple(std::string("return Module.call_cfunction(\"") + key + "\", [" + arg_array_str + "]);")));
        return ValBinding::toWireType(func);
    }

    static std::function<R(Args...)> fromWireType(WireType value) {
        // converter code
    }
};
}
