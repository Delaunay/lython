#include "ast/nodes.h"
#include "ast/ops.h"

namespace lython {

struct IsAttr {
    using Return = StmtNode*;

    IsAttr(String const& n): name(n) {}

    String const& name;

    static Return isattr(StmtNode* stmt, String const& name) { return IsAttr(name).lookup(stmt); }

    bool match(ExprNode* n) {
        if (n->kind == NodeKind::Name) {
            auto nn = cast<Name>(n);
            if (nn->id == name) {
                return true;
            }
        }
        return false;
    }
    Return exported(Exported* n) {
        return nullptr;
    }

    Return assign(Assign* n) {
        for (auto& target: n->targets) {
            if (match(target)) {
                return n;
            }
        }
        return nullptr;
    }
    Return annassign(AnnAssign* n) {
        if (match(n->target))
            return n;
        return nullptr;
    }

    Return functiondef(FunctionDef* n) {
        if (n->name == name) {
            return n;
        }
        return nullptr;
    }

    Return classdef(ClassDef* n) {
        if (n->name == name) {
            return n;
        }
        return nullptr;
    }

    Return lookup(StmtNode* obj) {
        switch (obj->kind) {
        case NodeKind::FunctionDef: {
            auto n = cast<FunctionDef>(obj);
            return functiondef(n);
        }
        case NodeKind::Assign: {
            auto n = cast<Assign>(obj);
            return assign(n);
        }
        case NodeKind::AnnAssign: {
            auto n = cast<AnnAssign>(obj);
            return annassign(n);
        }
        case NodeKind::ClassDef: {
            auto n = cast<ClassDef>(obj);
            return classdef(n);
        }

        default: break;
        }
        return nullptr;
    }
};

StmtNode* getattr(StmtNode* obj, String const& attr, ExprNode*& type) {
    if (obj->kind != NodeKind::ClassDef) {
        return nullptr;
    }

    ClassDef* def    = cast<ClassDef>(obj);
    int       attrid = def->get_attribute(attr);

    if (attrid > -1) {
        ClassDef::Attr& at = def->attributes[attrid];
        return at.stmt;
    }

    // Static Lookup
    for (auto& stmt: def->body) {
        auto value = IsAttr::isattr(stmt, attr);

        if (value != nullptr) {
            return value;
        }
    }

    return nullptr;
};

bool hasattr(StmtNode* obj, String const& attr) {
    ExprNode* dummy;
    return getattr(obj, attr, dummy) != nullptr;
}

}  // namespace lython