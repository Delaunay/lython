#include "expressions.h"
#include "nodes.h"

namespace lython {
std::ostream& Expression::print(std::ostream& out, int indent) const {
    _ptr->print(out);
    return out;
}

AST::NodeKind Expression::kind() const{
    return _ptr->kind();
}

namespace AST{
const char* to_string(NodeKind kind){
    switch(kind){
    case NodeKind::KArrow         : return "KindArrow";
    case NodeKind::KBuiltin       : return "KindBuiltin";
    case NodeKind::KParameter     : return "KindParameter";
    case NodeKind::KBinaryOperator: return "KindBinaryOperator";
    case NodeKind::KUnaryOperator : return "KindUnaryOperator";
    case NodeKind::KSeqBlock      : return "KindSeqBlock";
    case NodeKind::KFunction      : return "KindFunction";
    case NodeKind::KUnparsedBlock : return "KindUnparsedBlock";
    case NodeKind::KStatement     : return "KindStatement";
    case NodeKind::KValue         : return "KindValue";
    case NodeKind::KCall          : return "KindCall";
    case NodeKind::KReference     : return "KindReference";
    case NodeKind::KStruct        : return "KindStruct";
    case NodeKind::KType          : return "KindType";
    case NodeKind::KReversePolish : return "KindReversePolish";
    case NodeKind::KExternFunction: return "KindExternFunction";
    }
    return "<undefined>";
}

}
}
