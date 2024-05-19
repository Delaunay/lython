#include "lowering.h"

#include "utilities/magic.h"

namespace lython {

using StmtRet = Lowering::StmtRet;
using ExprRet = Lowering::ExprRet;
using ModRet  = Lowering::ModRet;
using PatRet  = Lowering::PatRet;


StmtRet Lowering::classdef(ClassDef_t* n, int depth) {
    for(int i = 0; i < n->body.size(); i++) {
        auto* stmt = n->body[i];

        if (FunctionDef* def = cast<FunctionDef>(stmt)) {

        }
    }
    return n;
}

}