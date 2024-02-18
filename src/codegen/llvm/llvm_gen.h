#ifndef LYTHON_LLVM_GEN_HEADER
#define LYTHON_LLVM_GEN_HEADER
#if WITH_LLVM && WITH_LLVM_CODEGEN 

#include "ast/magic.h"
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

struct LLVMGenVisitorTrait {
    using StmtRet = void;
    using ExprRet = llvm::Value*;
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
        for (std::size_t i = oldsize; i < array.size(); i++) {
            array[oldsize] = nullptr;
        }
        array.resize(oldsize);
    }

    Array<T>&   array;
    std::size_t oldsize;
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

    Array<llvm::Value*> named_values;
    Dict<Identifier, std::size_t> index_to_index;

    llvm::Type* builtin_type(StringRef name);
    llvm::Type* retrieve_type(ExprNode* type, int depth);

    llvm::BasicBlock* start_block = nullptr;
    llvm::BasicBlock* end_block = nullptr;

    //--
    Unique<llvm::legacy::FunctionPassManager> fun_optim;

#    if WITH_LLVM_DEBUG_SYMBOL
    Unique<llvm::DIBuilder> dbuilder;
    DICompileUnit*          debug_compile_unit;
    Arrayr<DIScope*>        scopes;

    void emit_location(ExprNode* node);
#    endif

    ExprRet
    binary_operator(llvm::IRBuilder<>* builder, ExprNode* left, ExprNode* right, int op, int depth);

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

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#    undef X
#    undef SSECTION
#    undef EXPR
#    undef STMT
#    undef MOD
#    undef MATCH
#    undef TYPE_GEN

#    define FUNCTION_GEN(name, fun, ret) ret fun(name##_t* n, int depth);

#    define X(name, _)
#    define SSECTION(name)
#    define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#    define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#    define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#    define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#    undef X
#    undef SSECTION
#    undef EXPR
#    undef STMT
#    undef MOD
#    undef MATCH

#    undef FUNCTION_GEN
};

}  // namespace lython

#endif
#endif