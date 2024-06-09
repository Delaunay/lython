#ifndef LYTHON_VM_EVAL_HEADER
#define LYTHON_VM_EVAL_HEADER

#include "utilities/printing.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/guard.h"
#include "utilities/strings.h"
#include "vm/tree.h"
#include "vm/vm.h"

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

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

struct Label {
    StmtNode* stmt = nullptr;
    String    name;
    int       index = -1;
    int       depth;
};


struct Program {
    Array<Instruction> instructions;
    Array<Label>       labels;

    int find_label(String const& name) {
        for(Label& l: labels) {
            if (l.name == name) {
                return l.index;
            }
        }
        return -1;
    }
};

struct LoopContext {
    Jump* breakjmp = nullptr;
    Jump* continuejmp = nullptr;
};


struct VariableIndex {
    int inserted_size;
    Name* name;
};

struct LazyCallResolve {
    Call* call = nullptr;
};

struct YieldedFunction {
    Array<Value>       variables;   //
    Array<Value>       registers;   //
    lython::StackTrace stacktrace;  //
    int ic = -1;                    // instruction counter to resume from
};

/**
 * The goal of this is to flatten the control flow into simple jumps
 * The code is flatten into a "tape" of instructions.
 * control flow change the instruction pointer
 *
 * This simplify resumption, while keeping the rest as high level as possible
 *
 * Technically call is a jump but it is unconditional so the CPU can cache the instruction ahead
 * 
 * Exceptions handling: when a try-catch is found it push exception handling on a global stack (records the handled exception type & the resume jump)
 * when raise is called it does an unconditional jump to an exception handling function that will
 * lookup for the exception specific handler.
 * 
 * cleanup code still needs to be triggered in the right order though
 * 
 */
struct VMGen: public BaseVisitor<VMGen, false, VMGenTrait> {
    using Super = BaseVisitor<VMGen, false, VMGenTrait>;

    struct GenContext {};

#define FUNCTION_GEN(name, fun) Value fun(name##_t* n, int depth);

    KW_FOREACH_AST(FUNCTION_GEN)

#undef FUNCTION_GEN

    Array<GenContext>       contexts;
    Array<Instruction>      program;
    Array<Label>            labels;
    Array<LoopContext>      loop_ctx;
    Array<LazyCallResolve>  calls_to_be_resolved;

    int instruction_counter() { return int(program.size()); }

    void add_instruction(VMNode* stmt) { 
        kwassert(stmt != nullptr, "Null instruction");
        program.push_back({stmt}); 
    }
    void add_instruction(StmtNode* stmt) {
        VMStmt* node = stmt->new_object<VMStmt>();
        node->stmt   = stmt;
        kwassert(node != nullptr, "Null instruction");
        program.push_back({node});
    }

    void add_body(String const& name, StmtNode* node, Array<StmtNode*> const& body, int depth) {
        labels.push_back({node, name, int(program.size()), depth});
        for (auto* stmt: body) {
            Super::exec(stmt, depth);
        }
    }
};

struct VMExec: public BaseVisitor<VMExec, false, VMGenTrait> {
    using Super = BaseVisitor<VMExec, false, VMGenTrait> ;

    // How to handle nested exception ?
    //
    enum class Registers: int {
        ReturnAddress,
        ReturnValue,
        Size,
    };

    int   ic = 0;

    VMExec():
        registers(int(Registers::Size))
    {
        // addressable variables which is different from reachable
        // addressable variables are always reachable
        // but reachable might not be addressable
        variables.reserve(128);
        args.reserve(8);
    }

    void set_program(Program const* prog) {
        program = prog;
    }

    Value execute(int entry);
    Value execute(Program const& program, int entry);
    void exec(int ic, int depth);

    template<typename... Args>
    Array<Value>& makeargs(Args... newargs) {
        args = {newargs...};
        return args;
    }

    Array<Value> args;
    Program const* program = nullptr;

    Value getreg(Registers register_) {
        return registers[int(register_)];
    }
    void setreg(Registers register_, Value val) {
        registers[int(register_)] = val;
    }

    int add_value(Value val) {
        int idx = len(variables);
        variables.push_back(val);
        return idx;
    }

    int compute_jump_call_address(Call_t* n, int depth); 

    void set_ic(int entry) {
        ic = entry;
        kwdebug(outlog(), "IC: {}", ic);
    }
    void inc_ic() {
        ic += 1;
        kwdebug(outlog(), "IC: {}", ic);
    }

    Array<Value>              variables;
    Array<Value>              registers;
    Array<lython::StackTrace> stacktrace;

#define FUNCTION_GEN(name, fun) Value fun(name##_t* n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)   FUNCTION_GEN(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)
#define VM(name, fun)    FUNCTION_GEN(name, fun)

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

inline Program compile(Module* mod) {
    VMGen compiler;
    compiler.exec(mod, 0);
    return Program{compiler.program, compiler.labels};
}

inline Value eval(Program const& program) {
    VMExec eval;
    return eval.execute(program, 0);
}

}  // namespace lython

#endif