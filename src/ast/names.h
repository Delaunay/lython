#ifndef LYTHON_SRC_AST_HEADER
#define LYTHON_SRC_AST_HEADER

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../Types.h"
#include "../logging/logging.h"


namespace lython {
// /!\ string a allocated twice

class Exception {
  public:
    Exception(String msg) : msg(std::move(msg)) {}
    const char *what() const noexcept { return msg.c_str(); }

  private:
    String msg;
};

#define ASSERT(pred, msg)                                                      \
    {                                                                          \
        if (!(pred)) {                                                         \
            throw Exception(msg);                                              \
        }                                                                      \
    }

// hold information about operators
struct OperatorImpl {
    OperatorImpl(const String& op_, int pred) : op(op_), pred(pred) {}

    const String op;
    const int pred;
};

/*
 *  Prevent the creation of two equivalent entities (strings)
 */
using Name = std::string_view;
using TypeName = std::string_view;
using Operator = std::shared_ptr<OperatorImpl>;

class NameManager {
  public:
    Name make_name(const String &name) {
        ASSERT(_types.count(name) == 0, "Name not available. Used by a Type");

        if (_names.count(name))
            return _names[name];

        _names[name] = name;
        return _names[name];
    }

    TypeName make_type(const String &type) {

        ASSERT(_names.count(type) == 0,
               "Type name not available. Used by a Name");

        if (_types.count(type))
            return _types[type];

        _types[type] = TypeName(type);
        return _types[type];
    }

    Operator make_operator(const String &op, int pred) {

        if (_operators.count(op))
            return _operators[op];

        _operators[op] = std::make_shared<OperatorImpl>(op, pred);
        return _operators[op];
    }

  private:
    std::unordered_map<String, Name> _names;
    std::unordered_map<String, TypeName> _types;
    std::unordered_map<String, Operator> _operators;
};

inline NameManager &name_manager() {
    static NameManager m;
    return m;
}

// use to create or retrieve a name
inline String make_name(const String &name) {
    return name; // name_manager().make_name(name);
}

inline String make_type(const String &name) {
    return name; // name_manager().make_type(name);
}

inline Operator make_operator(const String &name, int pred) {
    return name_manager().make_operator(name, pred);
}

/*
Type make_type(const String& type){
    if (name_manager().count(type) != 0)
        assert("Type Already Exists");

    return name_manager().make_name(type);
}

Type use_type(const String& type){
    if (name_manager().count(type) == 0)
        assert("Type Does not Exists");

    return name_manager().make_name(type);
}

Type make_type_weak(const String& name){
    return name_manager().make_name(name);
}*/
}

#endif
