#include "value.h"

namespace lython {


String to_string(ValueKind kind){
    switch (kind){
    #define X(type)\
        case ValueKind::pod_##type: return #type;
        POD_TYPES(X)
    #undef X

    case ValueKind::pod_str:
        return "str";
    case ValueKind::obj_closure:
        return "closure";
    case ValueKind::obj_object:
        return "object";
    case ValueKind::obj_class:
        return "class";
    case ValueKind::obj_none:
        return "null";
    }
    return "<not_implemented>";
}

Value::Value(AST::Function const* fun, Array<Value>  env):
    tag(ValueKind::obj_closure),
    _obj(std::make_shared<value::Closure>(fun, std::move(env), nullptr))
{
}

Value::Value(BuiltinImpl fun, Array<Value>  env):
    tag(ValueKind::obj_closure),
    _obj(std::make_shared<value::Closure>(nullptr, std::move(env), std::move(fun)))
{}

Value::Value(AST::Struct const* cstruct):
    tag(ValueKind::obj_class),
    _obj(std::make_shared<value::Class>(cstruct))
{}


value::Closure* Value::get_closure(){
    if (tag != ValueKind::obj_closure)
        return nullptr;

    return static_cast<value::Closure*>(_obj.get());
}

Value new_object(){
    Value value(ValueKind::obj_object);
    value._obj = std::make_shared<value::Struct>();
    return value;
}

}
