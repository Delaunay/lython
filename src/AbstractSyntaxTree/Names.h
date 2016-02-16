#pragma once

#include <cassert>

#include <unordered_map>
#include <string>
#include <memory>

namespace lython{
// /!\ string a allocated twice

// hold information about operators
struct OperatorImpl{
    OperatorImpl(const std::string op_, int pred):
        op(op_), pred(pred)
    {}

    const std::string op;
    const int pred;
};

/*
 *  Prevent the creation of two equivalent entities (strings)
 */
typedef std::shared_ptr<const std::string> Name;
typedef std::shared_ptr<const std::string> Type;
typedef std::shared_ptr<OperatorImpl> Operator;
 
class NameManager
{
    public:
    
    Name make_name(const std::string& name){
        
        assert(_types.count(name) == 0 && "Name not available. Used by a Type");
    
        if (_names.count(name))
            return _names[name];
        
        _names[name] = Name(new std::string(name));
        return _names[name];
    }
    
    Type make_type(const std::string& type){
        
        assert(_names.count(type) == 0 && "Type name not available. Used by a Name");

        if (_types.count(type))
            return _types[type];
        
        _types[type] = Type(new std::string(type));
        return _types[type];
    }

    Operator make_operator(const std::string& op, int pred){

        if (_operators.count(op))
            return _operators[op];

        _operators[op] = Operator(new OperatorImpl(op, pred));
        return _operators[op];
    }
    
    private:
        
        std::unordered_map<std::string, Name> _names;
        std::unordered_map<std::string, Type> _types;
        std::unordered_map<std::string, Operator> _operators;
};

NameManager& name_manager(){
    static NameManager m;
    return m;
}

// use to create or retrieve a name
Name make_name(const std::string& name){
    return name_manager().make_name(name);
}

Type make_type(const std::string& name){
    return name_manager().make_type(name);
}

Operator make_operator(const std::string& name, int pred){
    return name_manager().make_operator(name, pred);
}

/*
Type make_type(const std::string& type){
    if (name_manager().count(type) != 0)
        assert("Type Already Exists");
        
    return name_manager().make_name(type);
}

Type use_type(const std::string& type){
    if (name_manager().count(type) == 0)
        assert("Type Does not Exists");
        
    return name_manager().make_name(type);
}

Type make_type_weak(const std::string& name){
    return name_manager().make_name(name);
}*/

}
