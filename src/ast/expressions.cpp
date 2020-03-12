#include "expressions.h"
#include "nodes.h"
#include "print.h"

namespace lython {
std::ostream& Expression::print(std::ostream& out, int indent, bool dbg, bool color) const {
    lython::print(out, *this, indent, dbg, color);
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

Token& Expression::start() {    return _ptr->start; }
Token& Expression::end  () {    return _ptr->end; }

Token const& Expression::start() const {    return _ptr->start; }
Token const& Expression::end  () const {    return _ptr->end;   }
}
