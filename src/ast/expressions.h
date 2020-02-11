#ifndef LYTHON_EXPR_HEADER
#define LYTHON_EXPR_HEADER
#include <memory>

namespace lython {

namespace AST{
class Node;

// Explicit RTTI
enum class NodeKind {
    KArrow,
    KBuiltin,
    KParameter,
    KBinaryOperator,
    KUnaryOperator,
    KSeqBlock,
    KFunction,
    KUnparsedBlock,
    KStatement,
    KValue,
    KCall,
    KReference,
    KStruct,
    KType,
    KReversePolish,
    KExternFunction
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
        std::shared_ptr<AST::Node> ptr = std::make_shared<T>(std::forward<Args>(args)...);
        return Expression(ptr);
    }

    //! Returns a non owning pointer to the underlying Node
    template<typename T>
    T* ref(){
        // assert(kind() == kind_type<T>())
        return static_cast<T*>(_ptr.get());
    }

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
    operator bool(){    return bool(_ptr); }

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
