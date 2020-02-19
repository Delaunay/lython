#ifndef LYTHON_EXPR_HEADER
#define LYTHON_EXPR_HEADER

#include "utilities/allocator.h"

namespace lython {

namespace AST{
class Node;

#define NODE_KIND_ENUM(KIND)\
    KIND(Arrow, arrow)\
    KIND(Builtin, builtin)\
    KIND(Parameter, parameter)\
    KIND(BinaryOperator, binary)\
    KIND(UnaryOperator, unary)\
    KIND(SeqBlock, sequential)\
    KIND(Function, function)\
    KIND(UnparsedBlock, unparsed)\
    KIND(Statement, statement)\
    KIND(Value, value)\
    KIND(Call, call)\
    KIND(Reference, reference)\
    KIND(Struct, struct_type)\
    KIND(Type, type)\
    KIND(ReversePolish, reverse_polish)\
    KIND(ExternFunction, extern_function)

// Explicit RTTI
enum class NodeKind {
    KUndefined,
#define KIND(name, _) K##name,
    NODE_KIND_ENUM(KIND)
#undef KIND
};

//! Print the name of a Node Kind
const char* to_string(AST::NodeKind kind);
}


//! Generic Public Handle that restore value semantic
class Expression{
public:
    Expression() = default;

    template<typename T, typename ... Args>
    static Expression make(Args&& ... args){
        std::shared_ptr<AST::Node> ptr = lython::make_shared<T>(std::forward<Args>(args)...);
        return Expression(ptr);
    }

    //! Returns the underlying data struct held by the node
    template<typename T>
    T* ref(){
        // assert(kind() == kind_type<T>())
        return static_cast<T*>(_ptr.get());
    }

    //! Returns the underlying data struct held by the node
    template<typename T>
    T const* ref() const {
        // assert(kind() == kind_type<T>())
        return static_cast<T const*>(_ptr.get());
    }

    //! Returns the kind of the held AST Node
    AST::NodeKind kind() const;

    //! Prints the AST node
    std::ostream& print(std::ostream& out, int indent = 0) const;

    //! Returns true if the Expression holds a valid AST node
    operator bool() const {    return bool(_ptr); }

private:
    Expression(std::shared_ptr<AST::Node> ptr):
        _ptr(ptr)
    {}

    std::shared_ptr<AST::Node> _ptr;
};

inline
std::ostream& operator<<(std::ostream& out, Expression expr){
    expr.print(out);
    return out;
}

}
#endif
