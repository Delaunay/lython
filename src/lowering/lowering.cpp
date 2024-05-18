#include "lowering.h"

#include "utilities/magic.h"

namespace lython {

using StmtRet = Lowering::StmtRet;
using ExprRet = Lowering::ExprRet;
using ModRet  = Lowering::ModRet;
using PatRet  = Lowering::PatRet;


StmtRet Lowering::classdef(ClassDef_t* n, int depth) {
    return n;
}

}