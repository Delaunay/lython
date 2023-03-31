// include
#include "codegen/llvm/llvm_gen.h"

// Kiwi
#include "ast/magic.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

// STL
#include <iostream>

#if WITH_LLVM_CODEGEN

// LLVM
#    include "llvm/ADT/STLExtras.h"
#    include "llvm/Analysis/BasicAliasAnalysis.h"
#    include "llvm/Analysis/Passes.h"
#    include "llvm/IR/DIBuilder.h"
#    include "llvm/IR/IRBuilder.h"
#    include "llvm/IR/LLVMContext.h"
#    include "llvm/IR/LegacyPassManager.h"
#    include "llvm/IR/Module.h"
#    include "llvm/IR/Verifier.h"
#    include "llvm/Support/Host.h"
#    include "llvm/Support/TargetSelect.h"
#    include "llvm/Transforms/Scalar.h"

namespace lython {

using StmtRet = LLVMGen::StmtRet;
using ExprRet = LLVMGen::ExprRet;
using ModRet  = LLVMGen::ModRet;
using PatRet  = LLVMGen::PatRet;

using namespace llvm;

const char* tostr(StringRef const& ref) { return String(ref).c_str(); }


void LLVMGen::dump() {
    llmodule->print(llvm::outs(), nullptr);
}

LLVMGen::LLVMGen() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    context  = std::make_unique<LLVMContext>();
    llmodule = std::make_unique<llvm::Module>("KiwiJIT", *context);
    builder  = std::make_unique<IRBuilder<>>(*context);

#    if WITH_LLVM_DEBUG_SYMBOL
    dbuilder           = std::make_unique<DIBuilder>(*llmodule);
    String sourcefile  = "<string>";
    debug_compile_unit = dbuilder->createCompileUnit(
        dwarf::DW_LANG_C, dbuilder->createFile(tostr(sourcefile), "."), "Kiwi Compiler", 0, "", 0);
#    endif

    // TODO put this somewhere
    // dbuilder->finalize();
}

LLVMGen::~LLVMGen() {
    // llvm::llvm_shutdown();
}

#    if WITH_LLVM_DEBUG_SYMBOL
void LLVMGen::emit_location(ExprNode* node) {
    DIScope* scope;

    if (scopes.empty())
        scope = debug_compile_unit;
    else
        scope = scopes.back();

    builder.SetCurrentDebugLocation(
        DILocation::get(scope->getContext(), node->lineno, node->col_offset, scope));
}
#    endif

ExprRet LLVMGen::call(Call_t* n, int depth) {
    // Struct construction
    // std::vector<llvm::Constant*> values;
    // values.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 42));
    // values.push_back(llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), 3.14));
    // llvm::Constant* myStructInstance = llvm::ConstantStruct::get(struct_type, values);

    Value*              function = exec(n->func, depth);
    llvm::FunctionType* ftype    = nullptr;

    FunctionCallee callee(ftype, function);

    Array<Value*> args;
    for (size_t i = 0, end = n->args.size(); i != end; ++i) {
        ExprRet argvalue = exec(n->args[i], depth);
        args.push_back(argvalue);

        if (argvalue == nullptr) {
            kwerror("Could not generate function call");
            return nullptr;
        }
    }

    return builder->CreateCall(callee, args, "calltmp");
}

using BuiltinBinaryOperators = Dict<String, std::function<Value*(IRBuilder<>*, Value*, Value*)>>;

#    define LLMV_OPERATORS(OP) \
        OP(FAdd)               \
        OP(FSub)               \
        OP(FMul)               \
        OP(FCmp)

Value* fadd(IRBuilder<>* builder, Value* left, Value* right) {
    return builder->CreateFAdd(left, right, "addtmp");
}
Value* fsub(IRBuilder<>* builder, Value* left, Value* right) {
    return builder->CreateFSub(left, right, "addtmp");
}
Value* fmul(IRBuilder<>* builder, Value* left, Value* right) {
    return builder->CreateFMul(left, right, "addtmp");
}

// BuiltinBinaryOperators const& builtin_binary_operators() {
//     static BuiltinBinaryOperators ops = {
//         {String("f+"), fadd},
//         {String("f-"), fsub},
//         {String("f*"), fmul},
//     };
//     return ops;
// }

// Dict<String, std::function<Value*(Value*, Value*)>> operators

ExprRet LLVMGen::binary_operator(
    IRBuilder<>* builder, ExprNode* lefths, ExprNode* righths, int opidx, int depth) {
    Value* left  = exec(lefths, depth);
    Value* right = exec(righths, depth);

    if (left == nullptr || right == nullptr) {
        kwerror("Could not generate binary operator");
        return nullptr;
    }

    return builder->CreateFAdd(left, right, "addtmp");
}

ExprRet LLVMGen::binop(BinOp_t* n, int depth) {
    // Generic case is function call to a kiwi method/function
    //
    return binary_operator(builder.get(), n->left, n->right, 0, depth);
}
ExprRet LLVMGen::boolop(BoolOp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::unaryop(UnaryOp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::compare(Compare_t* n, int depth) { return ExprRet(); }

ExprRet LLVMGen::namedexpr(NamedExpr_t* n, int depth) {
    ExprRet target = exec(n->target, depth);
    ExprRet value  = exec(n->value, depth);
    return value;
}
ExprRet LLVMGen::lambda(Lambda_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::ifexp(IfExp_t* n, int depth) {
    Value* cond = exec(n->test, depth);

    if (cond == nullptr) {
        return nullptr;
    }

    Value* condcmp =
        builder->CreateFCmpONE(cond, ConstantFP::get(*context, APFloat(0.0)), "ifstmt_cond");

    Function* fundef = builder->GetInsertBlock()->getParent();

    BasicBlock* then   = BasicBlock::Create(*context, "then", fundef);
    BasicBlock* elxpr  = BasicBlock::Create(*context, "else");
    BasicBlock* merged = BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(condcmp, then, elxpr);

    // then
    builder->SetInsertPoint(then);
    Value* then_value = exec(n->body, depth);
    builder->CreateBr(merged);
    // ----

    then = builder->GetInsertBlock();
    fundef->getBasicBlockList().push_back(elxpr);

    // orelse
    builder->SetInsertPoint(elxpr);
    Value* else_value = exec(n->orelse, depth);
    builder->CreateBr(merged);
    // -----

    elxpr = builder->GetInsertBlock();

    fundef->getBasicBlockList().push_back(merged);
    builder->SetInsertPoint(merged);

    // Conditional value
    PHINode* cond_value = builder->CreatePHI(Type::getDoubleTy(*context), 2, "iftmp");
    cond_value->addIncoming(then_value, then);
    cond_value->addIncoming(else_value, elxpr);
    return cond_value;
}
ExprRet LLVMGen::dictexpr(DictExpr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::setexpr(SetExpr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::listcomp(ListComp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::generateexpr(GeneratorExp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::setcomp(SetComp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::dictcomp(DictComp_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::await(Await_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::yield(Yield_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::yieldfrom(YieldFrom_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::joinedstr(JoinedStr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::formattedvalue(FormattedValue_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::constant(Constant_t* n, int depth) {

    ConstantValue const& val = n->value;

    switch (n->value.get_kind()) {
    case ConstantValue::Ti8: return ConstantFP::get(*context, APFloat((float)val.get<int8>()));
    case ConstantValue::Ti16: return ConstantFP::get(*context, APFloat((float)val.get<int16>()));
    case ConstantValue::Ti32: return ConstantFP::get(*context, APFloat((float)val.get<int32>()));
    case ConstantValue::Ti64: return ConstantFP::get(*context, APFloat((float)val.get<int64>()));
    case ConstantValue::Tu8: return ConstantFP::get(*context, APFloat((float)val.get<uint8>()));
    case ConstantValue::Tu16: return ConstantFP::get(*context, APFloat((float)val.get<uint16>()));
    case ConstantValue::Tu32: return ConstantFP::get(*context, APFloat((float)val.get<uint32>()));
    case ConstantValue::Tu64: return ConstantFP::get(*context, APFloat((float)val.get<uint64>()));
    case ConstantValue::Tf32: return ConstantFP::get(*context, APFloat((float)val.get<float32>()));
    case ConstantValue::Tf64: return ConstantFP::get(*context, APFloat((float)val.get<float64>()));
    case ConstantValue::TBool: return ConstantFP::get(*context, APFloat((float)val.get<int8>()));
    }

    // TODO: object are a bit more complex

    return nullptr;
}
ExprRet LLVMGen::attribute(Attribute_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::subscript(Subscript_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::starred(Starred_t* n, int depth) { return ExprRet(); }

ExprRet LLVMGen::name(Name_t* n, int depth) {
    Value* value = named_values[n->id];

    if (value == nullptr) {

        return nullptr;
    }

#    if WITH_LLVM_DEBUG_SYMBOL
    KSDbgInfo.emitLocation(this);
#    endif

    // Load the value.
    return builder->CreateLoad(value, String(n->id).c_str());
}

ExprRet LLVMGen::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::tupleexpr(TupleExpr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::arrow(Arrow_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::comment(Comment_t* n, int depth) { return ExprRet(); }

StmtRet LLVMGen::functiondef(FunctionDef_t* n, int depth) {
    Array<Type*>        arg_types(n->args.size(), Type::getDoubleTy(*context));
    llvm::FunctionType* arrow =
        llvm::FunctionType::get(Type::getDoubleTy(*context), arg_types, false);

    Function* fundef = Function::Create(  //
        arrow,                            //
        Function::ExternalLinkage,        //
        tostr(n->name),                   //
        llmodule.get()                    //
    );
#    if WITH_LLVM_DEBUG_SYMBOL
    DIFile* unit =
        dbuilder->createFile(debug_compile_unit->getFilename(), debug_compile_unit->getDirectory());
    DIScope*      debug_ctx = unit;
    DISubprogram* debug_info =
        dbuilder->createFunction(debug_ctx,
                                 tostr(n->name),
                                 llvm::StringRef(),
                                 unit,
                                 n->lineno,
                                 CreateFunctionType(fundef->arg_size(), unit),
                                 false, /* internal linkage */
                                 true,  /* definition */
                                 0,
                                 DINode::FlagPrototyped,
                                 false);
    fundef->setSubprogram(debug_info);
#    endif

    unsigned i = 0;
    for (auto& arg: fundef->args()) {
        Identifier argname = n->args.args[i].arg;
        arg.setName(tostr(argname));

        /*
        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

        DILocalVariable *vard = dbuilder->createParameterVariable(
            SP,
            Arg.getName(),
            ++ArgIdx, Unit,
            LineNo,
            KSDbgInfo.getDoubleTy(),
            true
        );

        DBuilder->insertDeclare(Alloca, D, DBuilder->createExpression(),
                                DILocation::get(SP->getContext(), LineNo, 0, SP),
                                Builder.GetInsertBlock());

        // Store the initial value into the alloca.
        Builder.CreateStore(&Arg, Alloca);
        */

        i += 1;
    }

    BasicBlock* block = BasicBlock::Create(*context, "entry", fundef);
    builder->SetInsertPoint(block);

    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }

    verifyFunction(*fundef);

    return StmtRet();
}
StmtRet LLVMGen::classdef(ClassDef_t* n, int depth) {
    // Create the type definition
    Array<llvm::Type*> fields;
    llvm::Type*        int_type = llvm::Type::getInt32Ty(*context);
    fields.push_back(int_type);

    llvm::StructType::create(*context, fields, "my_struct");

    // TODO:
    // Runtime Reflection
    // Create Type Instance used for runtime reflextion

    return StmtRet();
}
StmtRet LLVMGen::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::returnstmt(Return_t* n, int depth) {

    if (n->value.has_value()) {
        Value* retvalue = exec(n->value.value(), depth);
        builder->CreateRet(retvalue);
        return;
    }

    return StmtRet();
}
StmtRet LLVMGen::deletestmt(Delete_t* n, int depth) {
    // Function call
    return StmtRet();
}

StmtRet LLVMGen::assign(Assign_t* n, int depth) {

    // Unpacking ?
    // create the variable
    Value* variable = exec(n->targets[0], depth);

    // Get the value
    Value* val = exec(n->value, depth);

    // Store the result
    builder->CreateStore(val, variable);
    return StmtRet();
}

StmtRet LLVMGen::augassign(AugAssign_t* n, int depth) {
    Value* variable = exec(n->target, depth);
    Value* val      = exec(n->value, depth);

    // Call the binary oerator here

    builder->CreateStore(val, variable);
    return StmtRet();
}
StmtRet LLVMGen::annassign(AnnAssign_t* n, int depth) {
    Value* variable = exec(n->target, depth);

    Value* val = nullptr;
    if (n->value.has_value()) {
        val = exec(n->value.value(), depth);
    }

    builder->CreateStore(val, variable);
    return StmtRet();
}

StmtRet LLVMGen::forstmt(For_t* n, int depth) {
    Function* fundef = builder->GetInsertBlock()->getParent();

    /*
    BasicBlock *body = BasicBlock::Create(*context, "loop", fundef);
    BasicBlock *orelse = BasicBlock::Create(*context, "orelse", fundef);
    BasicBlock *after = BasicBlock::Create(*context, "after", fundef);
    builder->CreateBr(body);

    builder->SetInsertPoint(body);
    for(auto* stmt: n->body) {
        exec(stmt, depth);
    }

    builder->SetInsertPoint(orelse);
    for(auto* stmt: n->orelse) {
        exec(stmt, depth);
    }


    builder->CreateCondBr(EndCond, body, after);
    builder->SetInsertPoint(after);
    */
    return StmtRet();
}
StmtRet LLVMGen::whilestmt(While_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::ifstmt(If_t* n, int depth) {
    Value* cond = exec(n->test, depth);

    if (cond == nullptr) {
        return StmtRet();
    }

    Value* condcmp =
        builder->CreateFCmpONE(cond, ConstantFP::get(*context, APFloat(0.0)), "ifstmt_cond");

    Function* fundef = builder->GetInsertBlock()->getParent();

    BasicBlock* then   = BasicBlock::Create(*context, "then", fundef);
    BasicBlock* elxpr  = BasicBlock::Create(*context, "else");
    BasicBlock* merged = BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(condcmp, then, elxpr);

    // then
    builder->SetInsertPoint(then);
    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }
    builder->CreateBr(merged);
    // ----

    then = builder->GetInsertBlock();
    fundef->getBasicBlockList().push_back(elxpr);

    // orelse
    builder->SetInsertPoint(elxpr);
    for (auto* stmt: n->orelse) {
        exec(stmt, depth);
    }
    builder->CreateBr(merged);
    // -----

    elxpr = builder->GetInsertBlock();

    fundef->getBasicBlockList().push_back(merged);
    builder->SetInsertPoint(merged);

    return StmtRet();
}
StmtRet LLVMGen::with(With_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::raise(Raise_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::trystmt(Try_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::assertstmt(Assert_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::global(Global_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::nonlocal(Nonlocal_t* n, int depth) { return StmtRet(); }

StmtRet LLVMGen::exprstmt(Expr_t* n, int depth) {
    exec(n->value, depth);
    return StmtRet();
}

StmtRet LLVMGen::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::breakstmt(Break_t* n, int depth) {
    // llvm::BranchInst::Create(exitblock, currentblock);
    // llvm::BasicBlock::getInstList().push_back()
    return StmtRet();
}
StmtRet LLVMGen::continuestmt(Continue_t* n, int depth) {
    // llvm::BranchInst::Create(loopBlock, currentBlock);
    // llvm::BasicBlock::getInstList().push_back()
    return StmtRet();
}
StmtRet LLVMGen::match(Match_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::inlinestmt(Inline_t* n, int depth) {
    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }
    return StmtRet();
}
StmtRet LLVMGen::import(Import_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::importfrom(ImportFrom_t* n, int depth) { return StmtRet(); }

PatRet LLVMGen::matchvalue(MatchValue_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchsingleton(MatchSingleton_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchsequence(MatchSequence_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchmapping(MatchMapping_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchclass(MatchClass_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchstar(MatchStar_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchas(MatchAs_t* n, int depth) { return PatRet(); }
PatRet LLVMGen::matchor(MatchOr_t* n, int depth) { return PatRet(); }

ModRet LLVMGen::module(Module_t* stmt, int depth) {
    for (auto* stmt: stmt->body) {
        exec(stmt, depth);
    }
    return ModRet();
};
ModRet LLVMGen::interactive(Interactive_t* n, int depth) { return ModRet(); }
ModRet LLVMGen::functiontype(FunctionType_t* n, int depth) { return ModRet(); }
ModRet LLVMGen::expression(Expression_t* n, int depth) {}

}  // namespace lython

#endif
