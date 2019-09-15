#ifndef LYTHON_SRC_AST_HEADER
#define LYTHON_SRC_AST_HEADER

#include <cassert>

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>


namespace lython{
// /!\ string a allocated twice

class Exception{
public:
    Exception(const std::string& msg):
        msg(msg)
    {}
    const char* what() const noexcept {  return msg.c_str(); }
private:
    std::string msg;
};

#define ASSERT(pred, msg) {if (!(pred)) {throw Exception(msg);}}

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
using Name = std::string_view;
using TypeName = std::string_view;
using Operator = std::shared_ptr<OperatorImpl>;
 

class NameManager
{
    public:
    
    Name make_name(const std::string& name){
        
        ASSERT(_types.count(name) == 0, "Name not available. Used by a Type");
    
        if (_names.count(name))
            return _names[name];
        
        _names[name] = name;
        return _names[name];
    }
    
    TypeName make_type(const std::string& type){
        
        ASSERT(_names.count(type) == 0, "Type name not available. Used by a Name");

        if (_types.count(type))
            return _types[type];
        
        _types[type] = TypeName(type);
        return _types[type];
    }

    Operator make_operator(const std::string& op, int pred){

        if (_operators.count(op))
            return _operators[op];

        _operators[op] = std::make_shared<OperatorImpl>(op, pred);
        return _operators[op];
    }
    
    private:
        
        std::unordered_map<std::string, Name> _names;
        std::unordered_map<std::string, TypeName> _types;
        std::unordered_map<std::string, Operator> _operators;
};

inline
NameManager& name_manager(){
    static NameManager m;
    return m;
}

// use to create or retrieve a name
inline
Name make_name(const std::string& name){
    return name_manager().make_name(name);
}

inline
TypeName make_type(const std::string& name){
    return name_manager().make_type(name);
}

inline
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

#endif
