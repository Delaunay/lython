#include "expressions.h"
#include "nodes.h"
#include "print.h"

namespace lython {
std::ostream& Expression::print(std::ostream& out, int indent) const {
    lython::print(out, *this, indent);
    return out;
}

AST::NodeKind Expression::kind() const{
    return _ptr->kind;
}

namespace AST{
const char* to_string(NodeKind kind){
    switch(kind){
    #define KIND(name, _)\
        case NodeKind::K##name: return "K"#name;
        NODE_KIND_ENUM(KIND)
    #undef KIND
    default:
        return "<undefined>";
    }
}

}
}
