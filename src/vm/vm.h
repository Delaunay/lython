#ifndef LYTHON_VM_EVAL_HEADER
#define LYTHON_VM_EVAL_HEADER

#include "ast/magic.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"
#include "utilities/guard.h"
#include "vm/vm.h"
#include "vm/tree.h"

namespace lython {

struct Instruction {
    // enum OpCode {
    //     pushscope,
    //     popscope,
    //     addvar,
    //     call,
    //     setvar,
    //     loadvar,
    //     jump,
    //     condjump
    // };

    // OpCode op;

    VMNode* stmt = nullptr;
};


struct VMGenTrait {
    using StmtRet = Value;
    using ExprRet = Value;
    using ModRet  = Value;
    using PatRet  = Value;
    using Trace   = std::true_type;

    enum { 
        MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH 
    };
};

struct Label {
    StmtNode* stmt = nullptr;
    String name;
    int index = -1;
    int depth;
};


/**
 * The goal of this is to flatten the control flow into simple jumps
 * The code is flatten into a "tape" of instructions.
 * control flow change the instruction pointer
 * 
 * This simplify resumption
 */
struct VMGen: public BaseVisitor<VMGen, false, VMGenTrait> 
{
    struct GenContext {

    };

    #define FUNCTION_GEN(name, fun)  Value fun(name##_t* n, int depth);

    #define X(name, _)
    #define SSECTION(name)
    #define MOD(name, fun)   FUNCTION_GEN(name, fun)
    #define EXPR(name, fun)  FUNCTION_GEN(name, fun)
    #define STMT(name, fun)  FUNCTION_GEN(name, fun)
    #define MATCH(name, fun) FUNCTION_GEN(name, fun)
    #define VM(name, fun)

        NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

    #undef X
    #undef SSECTION
    #undef EXPR
    #undef STMT
    #undef MOD
    #undef MATCH
    #undef VM

    #undef FUNCTION_GEN

    Array<GenContext>  contexts;
    Array<Instruction> program;
    Array<Label>       labels;

    int instruction_counter() {
        return int(program.size());
    }

    void add_instruction(VMNode* stmt) {
        program.push_back({stmt});
    }
    void add_instruction(StmtNode* stmt) {
        VMStmt* node = stmt->new_object<VMStmt>();
        node->stmt = stmt;
        program.push_back({node});
    }

    void add_body(String const& name, StmtNode* node, Array<StmtNode*> const& body, int depth) {
        labels.push_back({node, name, int(program.size()), depth});
        for(auto* stmt: body) {
            exec(stmt, depth);
        }
    }
};


struct VMExec: public BaseVisitor<VMExec, false, VMGenTrait> 
{
    int ic = 0;
    Value execute(Array<Instruction> const& program, int entry);

    Array<Value>      variable;
    Array<Value>      registers;
    Array<lython::StackTrace> stacktrace;

    #define FUNCTION_GEN(name, fun)  Value fun(name##_t* n, int depth);

    #define X(name, _)
    #define SSECTION(name)
    #define MOD(name, fun)   FUNCTION_GEN(name, fun)
    #define EXPR(name, fun)  FUNCTION_GEN(name, fun)
    #define STMT(name, fun)  FUNCTION_GEN(name, fun)
    #define MATCH(name, fun) FUNCTION_GEN(name, fun)
    #define VM(name, fun) FUNCTION_GEN(name, fun)

        NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

    #undef X
    #undef SSECTION
    #undef EXPR
    #undef STMT
    #undef MOD
    #undef MATCH
    #undef VM

    #undef FUNCTION_GEN

};

// Assemble programs together ?
// so we can process imported files in parallel
// and assemble them at the end
//
// Actually it is not too hard
// import with launch the compilation in parallel
// and make space in the main program to accomodate them
// we could probably put them at the end so we do not have to move too many addresses

inline Array<Instruction> compile(Module* mod) {
    VMGen compiler;
    compiler.exec(mod, 0);
    return compiler.program;
}

inline Value eval(Array<Instruction> program) {
    VMExec eval;
    return eval.execute(program, 0);
}

}

#endif