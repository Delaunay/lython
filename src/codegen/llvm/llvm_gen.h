#ifndef LYTHON_LLVM_GEN_HEADER
#define LYTHON_LLVM_GEN_HEADER

#ifndef WITH_LLVM
#define WITH_LLVM 1
#endif

#ifndef WITH_LLVM_CODEGEN
#define WITH_LLVM_CODEGEN 1
#endif


#if WITH_LLVM && WITH_LLVM_CODEGEN 

#include "utilities/printing.h"

#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"

// LLVM
#    include "llvm/IR/IRBuilder.h"
#    include "llvm/IR/LLVMContext.h"
#    include "llvm/IR/Module.h"
#    include "llvm/IR/LegacyPassManager.h"

namespace lython {

struct ValueOrType {
    union Holder {
        llvm::Value* value;
        llvm::Type* type;
    };

    Holder one_of;
    int tag;
  
    ValueOrType(): tag(-1) {}

    ValueOrType(std::nullptr_t t): tag(-1) {}

    ValueOrType(llvm::Value* value): tag(0) {
        one_of.value = value;
    }

    ValueOrType(llvm::Type* type): tag(1) {
        one_of.type = type;
    }
    llvm::Value* value() {
        if (tag == 0) {
            return one_of.value;
        }
        return nullptr;
    }
    llvm::Type* type() {
        if (tag == 1) {
            return one_of.type;
        }
        return nullptr;
    }
};

struct LLVMGenVisitorTrait {
    using StmtRet = void;
    using ExprRet = ValueOrType;
    using ModRet  = void;
    using PatRet  = void;
    using IsConst = std::false_type;
    using Trace   = std::true_type;

    enum {
        MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH
    };
};


template<typename T>
struct ArrayScope {
    ArrayScope(Array<T>& array): array(array), oldsize(array.size()) {
    }

    ~ArrayScope() {
        // for (std::size_t i = oldsize; i < array.size(); i++) {
        //     array[oldsize] = T();
        // }
        array.resize(oldsize);
    }

    Array<T>&   array;
    std::size_t oldsize;
};



struct VariableEntry {
    VariableEntry(Identifier name = Identifier(), 
                  llvm::Value* value = nullptr, 
                  llvm::Type* type = nullptr, 
                  bool is_alloca = false):
        name(name), value(value), type(type), is_alloca(is_alloca)
    {}

    Identifier   name;
    llvm::Value* value;
    llvm::Type*  type;
    bool         is_alloca = false;
};

// 
// X86
// WebAssembly
//
// SPIR-V => Graphics 
// WebGPU => WebGraphics >_<
// NVPTX  => Compute
// AMDGPU => Compute
// LLVM_EXPERIMENTAL_TARGETS_TO_BUILD=DirectX 
//
//  https://llvm.org/docs/DirectXUsage.html
//
//      arch-vendor-os
//      dxil-unknown-shadermodel 
//      spir-unknown-unknown
//      
//
//      Func->addFnAttr(Attribute::AttrKind::AmdgpuVertexShader);
//      builder.addExecutionMode(function, spv::ExecutionModeVertex);
/*
 */
struct LLVMGen: BaseVisitor<LLVMGen, false, LLVMGenVisitorTrait> {
    using Super = BaseVisitor<LLVMGen, false, LLVMGenVisitorTrait>;

    using StmtRet = Super::StmtRet;
    using ExprRet = Super::ExprRet;
    using ModRet  = Super::ModRet;
    using PatRet  = Super::PatRet;

    Unique<llvm::LLVMContext> context;
    Unique<llvm::Module>      llmodule;
    Unique<llvm::IRBuilder<>> builder;

    Array<VariableEntry> variables;
    Array<VariableEntry> types;

    // Array<llvm::Value*> named_values;
    // Dict<Identifier, std::size_t> index_to_index;

    llvm::Type* builtin_type(StringRef name);
    llvm::Type* retrieve_type(ExprNode* type, int depth);

    llvm::BasicBlock* start_block = nullptr;
    llvm::BasicBlock* end_block = nullptr;

    llvm::Value* binary_operator(BinaryOperator op, llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* make_condition(ExprNode* condition_expression, int depth, int i);
    //--
    Unique<llvm::legacy::FunctionPassManager> fun_optim;

#    if WITH_LLVM_DEBUG_SYMBOL
    Unique<llvm::DIBuilder> dbuilder;
    DICompileUnit*          debug_compile_unit;
    Arrayr<DIScope*>        scopes;
    void emit_location(ExprNode* node);
#    endif

    Logger& llvmlog = outlog();
    
    llvm::FunctionType* functiontype(Arrow_t* n, int depth);

    LLVMGen();
    ~LLVMGen();

    void dump() const;

#    define TYPE_GEN(rtype) using rtype##_t = Super::rtype##_t;

#    define X(name, _)
#    define SSECTION(name)
#    define EXPR(name, fun)  TYPE_GEN(name)
#    define STMT(name, fun)  TYPE_GEN(name)
#    define MOD(name, fun)   TYPE_GEN(name)
#    define MATCH(name, fun) TYPE_GEN(name)
#    define VM(name, fun) 


    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#    undef X
#    undef SSECTION
#    undef EXPR
#    undef STMT
#    undef MOD
#    undef MATCH
#    undef VM
#    undef TYPE_GEN

#    define FUNCTION_GEN(name, fun, ret) ret fun(name##_t* n, int depth);

#    define X(name, _)
#    define SSECTION(name)
#    define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#    define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#    define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#    define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)
#    define VM(name, fun) 

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#    undef X
#    undef SSECTION
#    undef EXPR
#    undef STMT
#    undef MOD
#    undef MATCH
#    undef VM

#    undef FUNCTION_GEN
};

}  // namespace lython

#endif
#endif