#include "value.h"
#include "ast/nodes.h"

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
    _obj(lython::make_shared<value::Closure>(fun, std::move(env), nullptr))
{
}

Value::Value(BuiltinImpl fun, Array<Value>  env):
    tag(ValueKind::obj_closure),
    _obj(lython::make_shared<value::Closure>(nullptr, std::move(env), std::move(fun)))
{}

Value::Value(AST::Struct const* cstruct):
    tag(ValueKind::obj_class),
    _obj(lython::make_shared<value::Class>(cstruct))
{}


value::Closure* Value::get_closure(){
    if (tag != ValueKind::obj_closure)
        return nullptr;

    return static_cast<value::Closure*>(_obj.get());
}

Value new_object(AST::Struct const*type){
    Value value(ValueKind::obj_object);
    value._obj = std::make_shared<value::Struct>(type);
    return value;
}


bool Value::operator==(Value const& v) const{
    if (v.tag != tag){
        return false;
    }

    switch(tag){
    #define X(type)\
        case ValueKind::pod_##type:{\
             return _data.v_##type == v._data.v_##type;\
        }
        POD_TYPES(X)
    #undef X

    default:
        return false;
    }
}

std::ostream& Value::print(std::ostream& out) const {
    switch (tag) {
#define X(type) case ValueKind::pod_##type:{\
                return out << #type << '(' << _data.v_##type << ')';\
            }
        POD_TYPES(X)
#undef X
    case ValueKind::pod_str:
        return out << "String(" << StringDatabase::instance()[_data.v_uint64] << ")";
    case ValueKind::obj_closure:
        return get<value::Closure const*>()->print(std::cout);
    case ValueKind::obj_class:
        return get<value::Class const*>()->print(std::cout);
    case ValueKind::obj_object:
        return get<value::Struct const*>()->print(std::cout);
    case ValueKind::obj_none:
        return out << "None";
    }
    return out;
}

std::ostream& value::Struct::print(std::ostream& out) const{
    out << type->name << "(";
    auto size = type->attributes.size();
    for(auto i = 0ul, n = size - 1; i < size; ++i){
        auto& name = std::get<0>(type->attributes[i]);
        out << name << '=';
        attributes.at(name).print(std::cout);

        if (i < n){
            out << ", ";
        }
    }
    return out << ")";
}

std::ostream& value::Class::print(std::ostream& out) const{
    return out << fun->name;
}

std::ostream& value::Closure::print(std::ostream& out) const{
    if (fun)
        return out << "Closure("<< fun->name << ")";
    return out << "Closure(bltin)";
}

}
