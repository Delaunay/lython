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
    case ValueKind::invalid:
        break;
    }
    return "<not_implemented>";
}

Value::Value(AST::Function const* fun, State* env):
    tag(ValueKind::obj_closure),
    _obj(lython::make_shared<value::Closure>(fun, env, nullptr))
{}

Value::Value(State& state, String const& name):
    tag(ValueKind::obj_module),
    _obj(lython::make_shared<value::Module>(state, name))
{
}

Value::Value(BuiltinImpl fun, State* env):
    tag(ValueKind::obj_closure),
    _obj(lython::make_shared<value::Closure>(nullptr, env, std::move(fun)))
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

std::ostream& Value::print(std::ostream& out, int depth) const {
    if (depth >= 8){
        throw std::runtime_error("Recursion error");
    }

    switch (tag) {
#define X(type) case ValueKind::pod_##type:{\
                return out << _data.v_##type;\
            }
        POD_TYPES(X)
#undef X
    case ValueKind::pod_str:
        return out << "String(" << StringDatabase::instance()[_data.v_uint64] << ")";
    case ValueKind::obj_closure:
        return get<value::Closure const*>()->print(out, depth + 1);
    case ValueKind::obj_class:
        return get<value::Class const*>()->print(out, depth + 1);
    case ValueKind::obj_object:
        return get<value::Struct const*>()->print(out, depth + 1);
    case ValueKind::obj_none:
        return out << "None";
    case ValueKind::invalid:
        return out << "<invalid>";
    }
    return out;
}

namespace value {
Struct::Struct(AST::Struct const* type):
    type(type), attributes(type->attributes.size())
{}

void Struct::set_attribute(StringRef name, Value val){
    set_attribute(type->offset.at(name), val);
}

void Struct::set_attribute(int idx, Value val){
    attributes[std::size_t(idx)] = val;
}

Value Struct::get_attributes(StringRef name){
    return get_attributes(type->offset.at(name));
}

Value Struct::get_attributes(int idx){
    return attributes[std::size_t(idx)];
}
}


std::ostream& value::Struct::print(std::ostream& out, int depth) const{
    out << type->name << "(";
    auto size = type->attributes.size();

    for(std::size_t i = 0ul, n = size - 1; i < size; ++i){
        auto& name = std::get<0>(type->attributes[i]);
        out << name << '=';
        attributes[i].print(out, depth + 1);

        if (i < n){
            out << ", ";
        }
    }
    return out << ")";
}

std::ostream& value::Class::print(std::ostream& out, int) const{
    return out << fun->name;
}

std::ostream& value::Closure::print(std::ostream& out, int) const{
    if (fun)
        return out << "Closure("<< fun->name << ")";

    if (builtin)
        return out << "Closure(bltin)";

    return out << "Closure(undef)";
}

}
