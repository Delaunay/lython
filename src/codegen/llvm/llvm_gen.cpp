// include
#include "codegen/llvm/llvm_gen.h"

#if WITH_LLVM && WITH_LLVM_CODEGEN
// Kiwi
#include "utilities/guard.h"
#include "utilities/printing.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

// STL
#include <iostream>

// LLVM
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

namespace lython {

using StmtRet = LLVMGen::StmtRet;
using ExprRet = LLVMGen::ExprRet;
using ModRet  = LLVMGen::ModRet;
using PatRet  = LLVMGen::PatRet;

using namespace llvm;

std::string tostr(StringRef const& ref) { return std::string(String(ref).c_str()); }

template <typename T>
std::string llvmstr(T* const obj) {
    std::string              str;
    llvm::raw_string_ostream ss(str);
    obj->print(ss);
    return str;
}

void LLVMGen::dump() const { llmodule->print(llvm::outs(), nullptr); }

LLVMGen::LLVMGen() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    context   = std::make_unique<LLVMContext>();
    llmodule  = std::make_unique<llvm::Module>("KiwiJIT", *context);
    builder   = std::make_unique<IRBuilder<>>(*context);
    fun_optim = std::make_unique<legacy::FunctionPassManager>(llmodule.get());

    fun_optim->add(llvm::createInstructionCombiningPass());
    fun_optim->add(llvm::createReassociatePass());
    fun_optim->add(llvm::createGVNPass());
    fun_optim->add(llvm::createCFGSimplificationPass());

    fun_optim->doInitialization();

#if WITH_LLVM_DEBUG_SYMBOL
    dbuilder           = std::make_unique<DIBuilder>(*llmodule);
    String sourcefile  = "<string>";
    debug_compile_unit = dbuilder->createCompileUnit(
        dwarf::DW_LANG_C, dbuilder->createFile(tostr(sourcefile), "."), "Kiwi Compiler", 0, "", 0);
#endif

    // TODO put this somewhere
    // dbuilder->finalize();
}

LLVMGen::~LLVMGen() {
    // llvm::llvm_shutdown();
}

#if WITH_LLVM_DEBUG_SYMBOL
void LLVMGen::emit_location(ExprNode* node) {
    DIScope* scope;

    if (scopes.empty())
        scope = debug_compile_unit;
    else
        scope = scopes.back();

    builder.SetCurrentDebugLocation(
        DILocation::get(scope->getContext(), node->lineno, node->col_offset, scope));
}
#endif

ExprRet LLVMGen::call(Call_t* n, int depth) {
    // Struct construction
    // std::vector<llvm::Constant*> values;
    // values.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 42));
    // values.push_back(llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), 3.14));
    // llvm::Constant* myStructInstance = llvm::ConstantStruct::get(struct_type, values);

    llvm::Value*              callee   = exec(n->func, depth).value();
    llvm::Function*           function = dyn_cast_or_null<llvm::Function>(callee);
    llvm::FunctionType const* ftype    = nullptr;

    if (function == nullptr) {
        kwerror(llvmlog, "Function is not callable");
        return nullptr;
    }
    ftype = function->getFunctionType();

    Array<llvm::Value*> args;
    for (size_t i = 0, end = n->args.size(); i != end; ++i) {
        llvm::Value* argvalue = exec(n->args[i], depth).value();
        args.push_back(argvalue);

        if (argvalue == nullptr) {
            kwerror(llvmlog, "Could not generate function call");
            return nullptr;
        }
    }

    kwassert(function->arg_size() == args.size(), "Argument should match");
    for (size_t i = 0; i < args.size(); i++) {
        auto* arg_type = ftype->getParamType((unsigned int)i);
        auto* val_type = args[i]->getType();

        if (arg_type != val_type) {
            kwerror(
                llvmlog, "Type mistmatch expected {} got {}", llvmstr(arg_type), llvmstr(val_type));
        }
    }

    return builder->CreateCall(function, args, "calltmp");
    // return builder->CreateCall(callee, args, "calltmp");
}

using BuiltinBinaryOperators =
    Dict<String, std::function<llvm::Value*(IRBuilder<>*, llvm::Value*, llvm::Value*)>>;

#define LLMV_OPERATORS(OP) \
    OP(FAdd)               \
    OP(FSub)               \
    OP(FMul)               \
    OP(FCmp)

llvm::Value* fadd(IRBuilder<>* builder, llvm::Value* left, llvm::Value* right) {
    return builder->CreateFAdd(left, right, "addtmp");
}
llvm::Value* fsub(IRBuilder<>* builder, llvm::Value* left, llvm::Value* right) {
    return builder->CreateFSub(left, right, "addtmp");
}
llvm::Value* fmul(IRBuilder<>* builder, llvm::Value* left, llvm::Value* right) {
    return builder->CreateFMul(left, right, "addtmp");
}

llvm::Value* LLVMGen::binary_operator(BinaryOperator op, llvm::Value* left, llvm::Value* right) {
    Type* type = left->getType();

    // clang-format: off
    if (type->isIntegerTy()) {
        switch (op) {
        case BinaryOperator::Add: return builder->CreateAdd(left, right, "addtmp");
        case BinaryOperator::Sub: return builder->CreateSub(left, right, "subtmp");
        case BinaryOperator::Mult: return builder->CreateMul(left, right, "multtmp");
        case BinaryOperator::Div:
            return builder->CreateSDiv(
                left, right, "divtmp");  // return builder->CreateUDiv(left, right, "divtmp");
        case BinaryOperator::Mod:
            return builder->CreateSRem(left, right);  // return builder->CreateURem(left, right);
        case BinaryOperator::FloorDiv:
            return builder->CreateSDiv(
                left, right, "sdivtmp");  // return builder->CreateUDiv(left, right, "");
        case BinaryOperator::Pow: {
            llvm::Function* powFunc =
                Intrinsic::getDeclaration(llmodule.get(), Intrinsic::pow, {type});
            return builder->CreateCall(powFunc, {left, right});
        }
        case BinaryOperator::LShift: return builder->CreateLShr(left, right, "addtmp");
        case BinaryOperator::RShift: return builder->CreateShl(left, right, "addtmp");
        case BinaryOperator::BitOr: return builder->CreateOr(left, right, "bitortmp");
        case BinaryOperator::BitXor: return builder->CreateXor(left, right, "bitxortmp");
        case BinaryOperator::BitAnd: return builder->CreateAnd(left, right, "bitandtmp");
        case BinaryOperator::MatMult: return nullptr;
        case BinaryOperator::EltDiv: return nullptr;
        case BinaryOperator::EltMult: return nullptr;
        }
    } else if (type->isFloatingPointTy()) {
        switch (op) {
        case BinaryOperator::Add: return builder->CreateFAdd(left, right, "addtmp");
        case BinaryOperator::Sub: return builder->CreateFSub(left, right, "subtmp");
        case BinaryOperator::Mult: return builder->CreateFMul(left, right, "multtmp");
        case BinaryOperator::Div: return builder->CreateFDiv(left, right, "divtmp");
        case BinaryOperator::Mod: return builder->CreateFRem(left, right);
        case BinaryOperator::Pow: {
            llvm::Function* powFunc =
                Intrinsic::getDeclaration(llmodule.get(), Intrinsic::pow, {type});
            return builder->CreateCall(powFunc, {left, right});
        }
        case BinaryOperator::FloorDiv: {
            // cast to int
            return builder->CreateFPToSI(builder->CreateFDiv(left, right, "sdivtmp"),
                                         IntegerType::get(*context, 32));
        }
        case BinaryOperator::MatMult: return nullptr;
        case BinaryOperator::EltDiv: return nullptr;
        case BinaryOperator::EltMult: return nullptr;
        case BinaryOperator::LShift: return nullptr;
        case BinaryOperator::RShift: return nullptr;
        case BinaryOperator::BitOr: return nullptr;
        case BinaryOperator::BitXor: return nullptr;
        case BinaryOperator::BitAnd: return nullptr;
        }
    }
    // clang-format: on

    kwerror(llvmlog, "binary operator not handled");
    return nullptr;
}

ExprRet LLVMGen::binop(BinOp_t* n, int depth) {
    llvm::Value* left  = exec(n->left, depth).value();
    llvm::Value* right = exec(n->right, depth).value();

    if (left == nullptr || right == nullptr) {
        kwerror(llvmlog, "Could not generate binary operator");
        return nullptr;
    }

    return binary_operator(n->op, left, right);
}
ExprRet LLVMGen::boolop(BoolOp_t* n, int depth) {

    Array<llvm::Value*> values(n->values.size());

    for (int i = 0; i < n->values.size(); i++) {
        values[i] = exec(n->values[i], depth).value();
    }

    auto fun = [this, n](llvm::Value* a, llvm::Value* b) -> llvm::Value* {
        switch (n->op) {
        case BoolOperator::Or: return builder->CreateOr(a, b, "ortmp");
        case BoolOperator::And: return builder->CreateAnd(a, b, "andtmp");
        }
        return nullptr;
    };

    // does this even matter ?
    // log2(op)
    while (values.size() >= 2) {
        int                 count = int(values.size()) / 2;
        int                 extra = int(values.size()) % 2;
        Array<llvm::Value*> next(count + extra);

        for (int i = 0; i < count; i++) {
            next[i] = fun(values[i * 2], values[i * 2 + 1]);
        }
        if (extra > 0) {
            next[count] = values[values.size() - 1];
        }
        values = next;
    }
    return values[0];
}

ExprRet LLVMGen::unaryop(UnaryOp_t* n, int depth) {
    llvm::Value* arg = exec(n->operand, depth).value();

    switch (n->op) {
    case UnaryOperator::Invert:
        return builder->CreateXor(arg, ConstantInt::get(arg->getType(), -1));
    case UnaryOperator::Not:
        // arg xor 1
        return builder->CreateXor(arg, ConstantInt::get(arg->getType(), 1));
    case UnaryOperator::UAdd: return arg;

    case UnaryOperator::USub:
        return builder->CreateSub(ConstantInt::get(arg->getType(), 0), arg, "subtmp");
    }
    return nullptr;
}
ExprRet LLVMGen::compare(Compare_t* n, int depth) {

    auto fun = [this](CmpOperator op, llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
        Type* type = lhs->getType();

        if (type->isIntegerTy()) {
            IntegerType* ITy = dyn_cast<IntegerType>(type);

            // if (ITy->isSigned())
            {
                // clang-format: off
                switch (op) {
                case CmpOperator::Eq: return builder->CreateICmpEQ(lhs, rhs, "");
                case CmpOperator::NotEq: return builder->CreateICmpNE(lhs, rhs, "");
                case CmpOperator::Lt: return builder->CreateICmpSLT(lhs, rhs, "");
                case CmpOperator::LtE: return builder->CreateICmpSLE(lhs, rhs, "");
                case CmpOperator::Gt: return builder->CreateICmpSGT(lhs, rhs, "");
                case CmpOperator::GtE: return builder->CreateICmpSGE(lhs, rhs, "");
                case CmpOperator::Is: return nullptr;
                case CmpOperator::IsNot: return nullptr;
                case CmpOperator::In: return nullptr;
                case CmpOperator::NotIn:
                    return nullptr;
                    // clang-format: on
                }
            }
        } else if (type->isFloatingPointTy()) {
            // clang-format: off
            switch (op) {
            case CmpOperator::Eq: return builder->CreateFCmp(CmpInst::FCMP_OEQ, lhs, rhs, "");
            case CmpOperator::NotEq: return builder->CreateFCmp(CmpInst::FCMP_UNE, lhs, rhs, "");
            case CmpOperator::Lt: return builder->CreateFCmpOLT(lhs, rhs, "");
            case CmpOperator::LtE: return builder->CreateFCmpOLE(lhs, rhs, "");
            case CmpOperator::Gt: return builder->CreateFCmpOGT(lhs, rhs, "");
            case CmpOperator::GtE: return builder->CreateFCmpOGE(lhs, rhs, "");
            case CmpOperator::Is: return nullptr;
            case CmpOperator::IsNot: return nullptr;
            case CmpOperator::In: return nullptr;
            case CmpOperator::NotIn:
                return nullptr;
                // clang-format: on
            }
        }
        return nullptr;
    };

    llvm::Value*        left = exec(n->left, depth).value();
    Array<llvm::Value*> comparisons;

    for (int i = 0; i < n->ops.size(); i++) {
        llvm::Value* right = exec(n->comparators[i], depth).value();
        comparisons.push_back(fun(n->ops[i], left, right));
        left = right;
    }

    llvm::Value* prev = comparisons[0];
    for (int i = 1; i < comparisons.size(); i++) {
        prev = builder->CreateAnd(prev, comparisons[i]);
    }

    return prev;
}

ExprRet LLVMGen::namedexpr(NamedExpr_t* n, int depth) {
    ExprRet target = exec(n->target, depth);
    ExprRet value  = exec(n->value, depth);

    // ?
    // builder->CreateStore(value, target);
    return value;
}
ExprRet LLVMGen::exported(Exported* n, int depth) { return nullptr; }
ExprRet LLVMGen::lambda(Lambda_t* n, int depth) {
    llvm::Function* lambdaFunc = llvm::Function::Create(nullptr,  // lambdaFuncType,
                                                        llvm::Function::ExternalLinkage,
                                                        "",
                                                        llmodule.get());

    return ExprRet();
}

llvm::Value* LLVMGen::make_condition(ExprNode* condition_expression, int depth, int i) {
    llvm::Value* condition = exec(condition_expression, depth).value();
    kwassert(condition != nullptr, "Condition cannot be empty");

    llvm::Value* condcmp = nullptr;
    Type*        type    = condition->getType();
    StringStream ss;
    ss << "cond_" << i;
    String str = ss.str();

    if (type->isIntegerTy()) {
        condcmp = builder->CreateICmpNE(  // Compare Not Equal
            condition,                    //
            ConstantInt::get(type, 0),    //
            str.c_str()                   //
        );
    } else if (type->isFloatingPointTy()) {
        condcmp = builder->CreateFCmpONE(  // Float Compare Ordered Not Equal
            condition,                     //
            ConstantFP::get(type, 0.0f),   //
            str.c_str()                    //
        );
    }

    kwassert(condcmp != nullptr, "Condition cannot be empty");
    return condcmp;
}

ExprRet LLVMGen::ifexp(IfExp_t* n, int depth) {
    llvm::Value*    condcmp = make_condition(n->test, depth, 0);
    llvm::Function* fundef  = builder->GetInsertBlock()->getParent();

    BasicBlock* then   = BasicBlock::Create(*context, "then", fundef);
    BasicBlock* elxpr  = BasicBlock::Create(*context, "else");
    BasicBlock* merged = BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(condcmp, then, elxpr);

    // then
    builder->SetInsertPoint(then);
    llvm::Value* then_value = exec(n->body, depth).value();
    builder->CreateBr(merged);
    // ----

    then = builder->GetInsertBlock();

    fundef->insert(fundef->end(), elxpr);
    // fundef->getBasicBlockList().push_back(elxpr);

    // orelse
    builder->SetInsertPoint(elxpr);
    llvm::Value* else_value = exec(n->orelse, depth).value();
    builder->CreateBr(merged);
    // -----

    elxpr = builder->GetInsertBlock();

    // fundef->getBasicBlockList().push_back(merged);
    fundef->insert(fundef->end(), merged);

    builder->SetInsertPoint(merged);

    // Conditional value
    PHINode* cond_value = builder->CreatePHI(else_value->getType(), 2, "iftmp");
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
ExprRet LLVMGen::placeholder(Placeholder_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::constant(Constant_t* n, int depth) {

    lython::Value& val = n->value;
    using Ty           = double;

    // clang-format off
    switch (meta::ValueTypes(n->value.tag)) {
    #if 0
    case meta::ValueTypes::i8:  return ConstantFP::get(*context, APFloat((Ty)val.get<int8>   ()));
    case meta::ValueTypes::i16: return ConstantFP::get(*context, APFloat((Ty)val.get<int16>  ()));
    case meta::ValueTypes::i32: return ConstantFP::get(*context, APFloat((Ty)val.get<int32>  ()));
    case meta::ValueTypes::i64: return ConstantFP::get(*context, APFloat((Ty)val.get<int64>  ()));
    case meta::ValueTypes::u8:  return ConstantFP::get(*context, APFloat((Ty)val.get<uint8>  ()));
    case meta::ValueTypes::u16: return ConstantFP::get(*context, APFloat((Ty)val.get<uint16> ()));
    case meta::ValueTypes::u32: return ConstantFP::get(*context, APFloat((Ty)val.get<uint32> ()));
    case meta::ValueTypes::u64: return ConstantFP::get(*context, APFloat((Ty)val.get<uint64> ()));
    case meta::ValueTypes::f32: return ConstantFP::get(*context, APFloat((Ty)val.get<float32>()));
    case meta::ValueTypes::f64: return ConstantFP::get(*context, APFloat((Ty)val.get<float64>()));
    case meta::ValueTypes::Bool:return ConstantFP::get(*context, APFloat((Ty)val.get<int8>   ()));
    #elif 0
    case meta::ValueTypes::i8:  return ConstantInt::get(*context, APInt(8,  (int8)   val.get<int8>   (), true));
    case meta::ValueTypes::i16: return ConstantInt::get(*context, APInt(16, (int16)  val.get<int16>  (), true));
    case meta::ValueTypes::i32: return ConstantInt::get(*context, APInt(32, (int32)  val.get<int32>  (), true));
    case meta::ValueTypes::i64: return ConstantInt::get(*context, APInt(64, (int64)  val.get<int64>  (), true));
    case meta::ValueTypes::u8:  return ConstantInt::get(*context, APInt(8,  (uint8)  val.get<uint8>  ()));
    case meta::ValueTypes::u16: return ConstantInt::get(*context, APInt(16, (uint16) val.get<uint16> ()));
    case meta::ValueTypes::u32: return ConstantInt::get(*context, APInt(32, (uint32) val.get<uint32> ()));
    case meta::ValueTypes::u64: return ConstantInt::get(*context, APInt(64, (uint64) val.get<uint64> ()));
    case meta::ValueTypes::f32: return ConstantFP ::get(*context, APFloat(  (float32)val.get<float32>()));
    case meta::ValueTypes::f64: return ConstantFP ::get(*context, APFloat(  (float64)val.get<float64>()));
    #else
    case meta::ValueTypes::i8:  return ConstantInt::get(IntegerType::get(*context,  8), (int8)   val.as<int8>   (), true);
    case meta::ValueTypes::i16: return ConstantInt::get(IntegerType::get(*context, 16), (int16)  val.as<int16>  (), true);
    case meta::ValueTypes::i32: return ConstantInt::get(IntegerType::get(*context, 32), (int32)  val.as<int32>  (), true);
    case meta::ValueTypes::i64: return ConstantInt::get(IntegerType::get(*context, 64), (int64)  val.as<int64>  (), true);
    case meta::ValueTypes::u8:  return ConstantInt::get(IntegerType::get(*context,  8), (uint8)  val.as<uint8>  ());
    case meta::ValueTypes::u16: return ConstantInt::get(IntegerType::get(*context, 16), (uint16) val.as<uint16> ());
    case meta::ValueTypes::u32: return ConstantInt::get(IntegerType::get(*context, 32), (uint32) val.as<uint32> ());
    case meta::ValueTypes::u64: return ConstantInt::get(IntegerType::get(*context, 64), (uint64) val.as<uint64> ());
    case meta::ValueTypes::f32: return ConstantFP ::get(Type::getFloatTy(*context)    , (float32)val.as<float32>());
    case meta::ValueTypes::f64: return ConstantFP ::get(Type::getDoubleTy(*context)   , (float64)val.as<float64>());
    case meta::ValueTypes::i1 : return ConstantInt::get(Type::getInt1Ty(*context)     , (bool)   val.as<bool>());
    case meta::ValueTypes::none:return nullptr;
    #endif
    }
    // clang-format on

    // TODO: object are a bit more complex
    // llvm::ConstantStruct::get();
    // llvm::ConstantArray

    kwassert(false, "Cannot generate null constant");
    return nullptr;
}
ExprRet LLVMGen::attribute(Attribute_t* n, int depth) {
    // struct lookup
    llvm::Value* obj = exec(n->value, depth).value();

    return builder->CreateStructGEP(
        dyn_cast<StructType>(obj->getType()), obj, n->attrid, str(n->attr).c_str());
}
ExprRet LLVMGen::subscript(Subscript_t* n, int depth) {
    // slice
    return ExprRet();
}
ExprRet LLVMGen::starred(Starred_t* n, int depth) {
    // unpacking wont exist anymore at that point right
    return ExprRet();
}

ExprRet LLVMGen::name(Name_t* n, int depth) {

    VariableEntry* found = nullptr;
    for (int i = int(variables.size()) - 1; i >= 0; --i) {
        VariableEntry& entry = variables[i];

        if (entry.name == n->id) {
            found = &entry;
        }
    }

    if (n->ctx == ExprContext::Store) {
        if (found != nullptr) {
            kwdebug(llvmlog, "Variable name found");
            return found->value;
        }

        // Variable does not exist and it is a store
        if (found == nullptr) {
            kwdebug(llvmlog, "Variable name not found, creating");
            auto* insert_block = builder->GetInsertBlock();
            auto* type         = retrieve_type(n->type, depth);

            if (insert_block == nullptr) {
                // Global variable
                GlobalVariable* variable = new llvm::GlobalVariable(
                    *llmodule.get(),  // Module to add the global variable to
                    type,             // Type of the global variable
                    false,            // Whether the variable is constant (false for mutable)
                    GlobalValue::ExternalLinkage,  // Linkage type
                    ConstantInt::get(type, 0),     // Initial value (optional)
                    str(n->id).c_str()             // Name of the global variable
                );
                variables.emplace_back(  //
                    n->id,
                    variable,
                    type,
                    true);
                return variable;
            }

            llvm::Function* fundef = insert_block->getParent();

            IRBuilder<> allocabuilder(&fundef->getEntryBlock(), fundef->getEntryBlock().begin());

            AllocaInst* variable = allocabuilder.CreateAlloca(  //
                type,                                           // Type
                nullptr,                                        // Value *ArraySize = nullptr
                tostr(n->id)                                    // Name
            );

            variables.emplace_back(  //
                n->id,
                variable,
                type,
                true);
            return variable;
        }
    }

    if (found == nullptr) {
        return nullptr;
    }

#if WITH_LLVM_DEBUG_SYMBOL
    KSDbgInfo.emitLocation(this);
#endif

    llvm::Type* type = found->type;
    if (type == nullptr) {
        type = found->value->getType();
    }
    // This is a function call
    if (type && !type->isSized()) {
        return found->value;
    }

    // Load the value.
    if (found->is_alloca) {
        return builder->CreateLoad(found->type, found->value, tostr(n->id));
    }
    return found->value;
}

ExprRet LLVMGen::listexpr(ListExpr_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::tupleexpr(TupleExpr_t* n, int depth) {
    // make a fake struct

    Array<llvm::Type*> fields;
    llvm::Type*        int_type = llvm::Type::getInt32Ty(*context);

    for (auto* type: n->type->types) {
        fields.push_back(retrieve_type(type, depth));
    }

    StructType* clstype = llvm::StructType::create(*context, fields, "");
    llmodule->getOrInsertGlobal("", clstype);

    AllocaInst* structInst = builder->CreateAlloca(clstype);

    for (int i = 0; i < n->elts.size(); i++) {
        llvm::Value* member   = builder->CreateStructGEP(clstype, structInst, i);
        llvm::Value* newvalue = exec(n->elts[i], depth).value();
        builder->CreateStore(newvalue, member);
    }
    return structInst;
}
ExprRet LLVMGen::slice(Slice_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::dicttype(DictType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::arraytype(ArrayType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::arrow(Arrow_t* n, int depth) { return ExprRet(); }

llvm::FunctionType* LLVMGen::functiontype(Arrow_t* n, int depth) {

    Array<llvm::Type*> args_types;
    args_types.reserve(n->args.size());

    for (auto& arg: n->args) {
        args_types.push_back(retrieve_type(arg, depth));
    }

    llvm::Type* return_type = retrieve_type(n->returns, depth);

    llvm::FunctionType* arrow = llvm::FunctionType::get(  //
        return_type,                                      //
        args_types,                                       //
        false                                             // isVarArg
    );

    return arrow;
}
ExprRet LLVMGen::builtintype(BuiltinType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::tupletype(TupleType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::settype(SetType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::classtype(ClassType_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::comment(Comment_t* n, int depth) { return ExprRet(); }

StmtRet LLVMGen::functiondef(FunctionDef_t* n, int depth) {
    Array<Type*> arg_types(n->args.size(), Type::getDoubleTy(*context));

    llvm::FunctionType* arrow = functiontype(n->type, depth);

    // ExternalLinkage: The symbol can be referenced from other translation units.
    // InternalLinkage: The symbol is local to the translation unit and cannot be referenced from
    // other translation units.
    //      can reduce the symbol table size and improve compilation times.
    //      symbol visible to debuggers
    // PrivateLinkage: Similar to internal linkage, but the symbol cannot be exported to the object
    // file's symbol table.
    //      even smaller than InternalLinkage
    //      symbol NOT visible to debuggers
    // WeakAnyLinkage: A weak symbol that can be overridden by a strong symbol of the same name.
    //      Weak symbols are often used for optional features or plugin systems, where a default
    //      implementation is provided but can be replaced by a stronger implementation if
    //      available.
    // WeakODRLinkage: A weak symbol that can be overridden by a strong symbol of the same name from
    // a different translation unit.
    llvm::Function* fundef = llvm::Function::Create(  //
        arrow,                                        //
        llvm::Function::ExternalLinkage,              //
        tostr(n->name),                               //
        llmodule.get()                                //
    );

    variables.emplace_back(VariableEntry{n->name, fundef, arrow});

    BasicBlock* block = BasicBlock::Create(*context, "entry", fundef);
    builder->SetInsertPoint(block);

#if WITH_LLVM_DEBUG_SYMBOL
    DIFile* unit =
        dbuilder->createFile(debug_compile_unit->getFilename(), debug_compile_unit->getDirectory());
    DIScope*      debug_ctx  = unit;
    DISubprogram* debug_info = dbuilder->createFunction(  //
        debug_ctx,                                        //
        tostr(n->name),                                   //
        llvm::StringRef(),                                //
        unit,                                             //
        n->lineno,                                        //
        CreateFunctionType(fundef->arg_size(), unit),     //
        false,                                            // internal linkage
        true,                                             // definition
        0,                                                //
        DINode::FlagPrototyped,                           //
        false                                             //
    );
    fundef->setSubprogram(debug_info);
#endif

    unsigned                  i = 0;
    ArrayScope<VariableEntry> scope(variables);

    for (llvm::Argument& arg: fundef->args()) {
        Identifier argname_ref = n->args.args[i].arg;
        Type*      type        = arrow->params()[i];

        std::string argname = tostr(argname_ref);
        arg.setName(argname);

        // alloca are not necessary if we do not modify the arguments
        // but it is a common ppatern
        IRBuilder<> allocabuilder(&fundef->getEntryBlock(), fundef->getEntryBlock().begin());

        AllocaInst* arg_mem = allocabuilder.CreateAlloca(  //
            type,                                          //
            nullptr,                                       //
            arg.getName()                                  //
        );

        variables.emplace_back(argname_ref, arg_mem /*&arg*/, type, true);

#if WITH_LLVM_DEBUG_SYMBOL
        DILocalVariable* vard = dbuilder->createParameterVariable(
            SP, arg.getName(), ++ArgIdx, Unit, LineNo, KSDbgInfo.getDoubleTy(), true);

        dbuilder->insertDeclare(Alloca,
                                D,
                                dbuilder->createExpression(),
                                DILocation::get(SP->getContext(), LineNo, 0, SP),
                                Builder.GetInsertBlock());
#endif

        // Store the initial value into the alloca.
        builder->CreateStore(&arg, arg_mem);
        //*/

        i += 1;
    }

    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }

    // LLVM does not like empty basic block
    // it is the control flow optimization pass that need something
    // if (block->empty())
    { builder->CreateRetVoid(); }

    verifyFunction(*fundef);

    fundef->print(outs());

    fun_optim->run(*fundef);

    return StmtRet();
}

llvm::Type* LLVMGen::builtin_type(StringRef name) {
    Array<std::tuple<StringRef, llvm::Type*>> builtin_types = {
        {StringRef("i8"), IntegerType::get(*context, 8)},
        {StringRef("i16"), IntegerType::get(*context, 16)},
        {StringRef("i32"), IntegerType::get(*context, 32)},
        {StringRef("i64"), IntegerType::get(*context, 64)},

        {StringRef("u8"), IntegerType::get(*context, 8)},
        {StringRef("u16"), IntegerType::get(*context, 16)},
        {StringRef("u32"), IntegerType::get(*context, 32)},
        {StringRef("u64"), IntegerType::get(*context, 64)},

        {StringRef("f32"), Type::getFloatTy(*context)},
        {StringRef("f64"), Type::getDoubleTy(*context)},  //
        {StringRef("bool"), Type::getInt1Ty(*context)}
        //
    };

    for (auto& entry: builtin_types) {
        if (std::get<0>(entry) == name) {
            Type* type = std::get<1>(entry);
            kwassert(type != nullptr, "Type have to be known");
            return type;
        }
    }
    return nullptr;
}

llvm::Type* LLVMGen::retrieve_type(ExprNode* type, int depth) {
    if (type == nullptr) {
        return llvm::Type::getVoidTy(*context);
    }
    kwinfo(llvmlog, "{} {}", str(type), str(type->kind));

    switch (type->kind) {
    case NodeKind::BuiltinType: {
        return builtin_type(cast<BuiltinType>(type)->name);
    }
    case NodeKind::Name: {
        Name* name = cast<Name>(type);

        auto* builtin = builtin_type(name->id);
        if (builtin) {
            return builtin;
        }

        for (auto& type: types) {
            if (type.name == name->id) {
                kwassert(type.type != nullptr, "Type have to be known");
                return type.type;
            }
        }
    }
    }

    // THIS SHOULD BE UNREACHABLE
    return nullptr;
}

// StmtRet LLVMGen::condjump(CondJump_t* n, int depth) {
//     return StmtRet();
// }

StmtRet LLVMGen::classdef(ClassDef_t* n, int depth) {
    // Create the type definition
    Array<llvm::Type*> fields;
    llvm::Type*        int_type = llvm::Type::getInt32Ty(*context);

    for (ClassDef::Attr const& attr: n->attributes) {
        llvm::Type* field_type = retrieve_type(attr.type, depth);
        if (field_type == nullptr) {
            kwerror(llvmlog, "Could not find type for {}", str(attr.type));
        }
        fields.push_back(field_type);
    }

    StructType* clstype = llvm::StructType::create(*context, fields, tostr(n->name));
    llmodule->getOrInsertGlobal(tostr(n->name), clstype);

    // // Create a struct instance
    // AllocaInst *structInst = builder.CreateAlloca(structType);
    // Value *floatMember = builder.CreateStructGEP(structType, structInst, 0);
    // Value *intMember = builder.CreateStructGEP(structType, structInst, 1);

    // index_to_index[n->name] = named_values.size();
    // named_values.push_back(clstype);

    // TODO:
    // Runtime Reflection
    // Create Type Instance used for runtime reflextion

    return StmtRet();
}
StmtRet LLVMGen::invalidstmt(InvalidStatement_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::returnstmt(Return_t* n, int depth) {

    if (n->value.has_value()) {
        llvm::Value* retvalue = exec(n->value.value(), depth).value();

        if (retvalue != nullptr) {
            builder->CreateRet(retvalue);
        } else {
            builder->CreateRetVoid();
        }
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
    llvm::Value* variable = exec(n->targets[0], depth).value();

    // Get the value
    llvm::Value* val = exec(n->value, depth).value();

    // Store the result
    builder->CreateStore(val, variable);
    return StmtRet();
}

StmtRet LLVMGen::augassign(AugAssign_t* n, int depth) {
    // AUG does both load and store
    llvm::Value* variable = exec(n->target, depth).value();
    llvm::Value* val      = exec(n->value, depth).value();

    llvm::Value* original = builder->CreateLoad(val->getType(), variable);

    // Call the binary operator here
    llvm::Value* newval = binary_operator(n->op, original, val);

    builder->CreateStore(newval, variable);
    return StmtRet();
}
StmtRet LLVMGen::annassign(AnnAssign_t* n, int depth) {
    // Special case: global variable
    llvm::Value* variable = exec(n->target, depth).value();

    llvm::Value* val = nullptr;
    if (n->value.has_value()) {
        val = exec(n->value.value(), depth).value();
    }

    builder->CreateStore(val, variable);
    return StmtRet();
}

StmtRet LLVMGen::forstmt(For_t* n, int depth) {
    llvm::Function* fundef = builder->GetInsertBlock()->getParent();

    BasicBlock* preheader = builder->GetInsertBlock();
    BasicBlock* body      = BasicBlock::Create(*context, "loop", fundef);
    BasicBlock* orelse    = BasicBlock::Create(*context, "orelse", fundef);
    BasicBlock* after     = BasicBlock::Create(*context, "after", fundef);

    start_block = body;
    end_block   = after;

    builder->CreateBr(body);
    builder->SetInsertPoint(body);

    // Initialization
    // exec(n->target, depth);
    // PHINode *var = builder->CreatePHI(Type::getDoubleTy(*context), 2, VarName);
    // var->addIncoming(StartVal, preheader);

    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }

    // Step val
    //
    //

    // Comparison
    // EndCond = builder->CreateFCmpONE(EndCond, ConstantFP::get(*context, APFloat(0.0)),
    // "loopcond");
    BasicBlock* loop_end = builder->GetInsertBlock();

    // builder->CreateCondBr(EndCond, loop_end, after);

    // var->addIncoming(NextVar, loop_end);

    builder->SetInsertPoint(orelse);
    for (auto* stmt: n->orelse) {
        exec(stmt, depth);
    }

    // builder->CreateCondBr(EndCond, body, after);
    builder->SetInsertPoint(after);

    start_block = nullptr;
    end_block   = nullptr;
    return StmtRet();
}
StmtRet LLVMGen::whilestmt(While_t* n, int depth) {
    //

    //
    return StmtRet();
}

StmtRet LLVMGen::ifstmt(If_t* n, int depth) {
    llvm::Function* fundef = builder->GetInsertBlock()->getParent();

    // First if
    BasicBlock* then_br = BasicBlock::Create(*context, "if_0", fundef);
    BasicBlock* else_br = BasicBlock::Create(*context, "elif_0");
    BasicBlock* end_br  = BasicBlock::Create(*context, "ifend");

    {
        // Create the condition
        //  (n->test != 0)
        builder->CreateCondBr(make_condition(n->test, depth, 0), then_br, else_br);

        builder->SetInsertPoint(then_br);
        for (auto* stmt: n->body) {
            exec(stmt, depth);
        }

        // join regular executation afterwards by branchin past else
        builder->CreateBr(end_br);
        then_br = builder->GetInsertBlock();
        // ----
    }

    BasicBlock* start_bb = else_br;
    for (int i = 0; i < n->tests.size(); i++) {
        builder->SetInsertPoint(start_bb);
        fundef->insert(fundef->end(), start_bb);

        StringStream ss1;
        ss1 << "then_" << i + 1;
        StringStream ss2;
        ss2 << "elif_" << i + 1;

        then_br = BasicBlock::Create(*context, ss1.str().c_str());
        else_br = BasicBlock::Create(*context, ss2.str().c_str());

        builder->CreateCondBr(make_condition(n->tests[i], depth, i + 1), then_br, else_br);

        fundef->insert(fundef->end(), then_br);
        builder->SetInsertPoint(then_br);
        for (auto* stmt: n->body) {
            exec(stmt, depth);
        }

        // join regular executation afterwards by branchin past else
        builder->CreateBr(end_br);
        then_br  = builder->GetInsertBlock();
        start_bb = else_br;
        // ----
    }

    // orelse
    {
        // fundef->getBasicBlockList().push_back(elxpr);
        fundef->insert(fundef->end(), start_bb);

        // orelse
        builder->SetInsertPoint(start_bb);
        for (auto* stmt: n->orelse) {
            exec(stmt, depth);
        }

        builder->CreateBr(end_br);
        // -----
        else_br = builder->GetInsertBlock();  // <= What does this do
    }

    // We weill insert to this afterwards
    fundef->insert(fundef->end(), end_br);
    builder->SetInsertPoint(end_br);

    return StmtRet();
}
StmtRet LLVMGen::with(With_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::raise(Raise_t* n, int depth) {
    BasicBlock*     current_block = builder->GetInsertBlock();
    llvm::Function* fundef        = current_block->getParent();

    llvm::Function* raisefun = nullptr;
    // llvm::Function* raisefun = Intrinsic::getDeclaration(fundef->getParent(),
    // Intrinsic::eh_throw);
    llvm::Value* exception = nullptr;
    CallInst*    raise     = builder->CreateCall(raisefun, {exception});
    raise->setDoesNotReturn();

    // builder->GetInsertBlock()->getInstList().push_back(raise);
    raise->insertInto(current_block, current_block->end());

    return StmtRet();
}
StmtRet LLVMGen::trystmt(Try_t* n, int depth) {

    // Body

    BasicBlock* body      = BasicBlock::Create(*context, "body");
    BasicBlock* handlers  = BasicBlock::Create(*context, "catch");
    BasicBlock* orelse    = BasicBlock::Create(*context, "orelse");
    BasicBlock* finalbody = BasicBlock::Create(*context, "finalbody");

    // Landing Pad for exception handling
    Type* Int8PtrTy = PointerType::getUnqual(Type::getInt8Ty(*context));
    // Constant* NullPtr   = ConstantPointerNull::get(Int8PtrTy);

    LandingPadInst* LPInst = LandingPadInst::Create(Int8PtrTy, 1, "landingpad", handlers);
    // LPInst->addClause(NullPtr);

    // InvokeInst* InvokeInst = InvokeInst::Create(Func, NormalBB, CatchBB, Args, "invoke",
    // EntryBB);

    return StmtRet();
}
StmtRet LLVMGen::assertstmt(Assert_t* n, int depth) {
    // raise an error
    llvm::Value* test = exec(n->test, depth).value();

    // BasicBlock* merged = BasicBlock::Create(*context, "ifcont");
    // llvm::Value* condcmp = builder->CreateFCmpONE(
    //     test,
    //     ConstantFP::get(*context, APFloat(0.0)),
    //     "ifstmt_cond"
    // );
    // BasicBlock* merged = BasicBlock::Create(*context, "assert_failed");

    // builder->CreateCondBr(condcmp, then, elxpr);

    // then
    // builder->SetInsertPoint(then);

    // The unreachable instruction indicates to the compiler
    // that the current code path is unreachable, meaning that
    // it should never be executed under any circumstances
    //
    // Not good because not catchable, only used for optimization
    //
    // builder->CreateUnreachable();

    // indicate an unrecoverable error
    //
    // This is not good too because not catchable as well
    //
    // builder->CreateIntrinsic(Intrinsic::trap, {}, {});

    return StmtRet();
}
StmtRet LLVMGen::global(Global_t* n, int depth) {
    //
    return StmtRet();
}
StmtRet LLVMGen::nonlocal(Nonlocal_t* n, int depth) {
    //
    return StmtRet();
}

StmtRet LLVMGen::exprstmt(Expr_t* n, int depth) {
    exec(n->value, depth);
    return StmtRet();
}

StmtRet LLVMGen::pass(Pass_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::breakstmt(Break_t* n, int depth) {
    BasicBlock* current_block = builder->GetInsertBlock();
    BranchInst* breakbr       = llvm::BranchInst::Create(end_block, current_block);

    // current_block->getInstList().push_back(breakbr);
    breakbr->insertInto(current_block, current_block->end());

    return StmtRet();
}

StmtRet LLVMGen::continuestmt(Continue_t* n, int depth) {
    BasicBlock* current_block = builder->GetInsertBlock();
    BranchInst* continuebr    = llvm::BranchInst::Create(start_block, current_block);
    // current_block->getInstList().push_back(continuebr);
    continuebr->insertInto(current_block, current_block->end());

    return StmtRet();
}

StmtRet LLVMGen::inlinestmt(Inline_t* n, int depth) {
    for (auto* stmt: n->body) {
        exec(stmt, depth);
    }
    return StmtRet();
}

StmtRet LLVMGen::import(Import_t* n, int depth) {
    // schedule module for code gen
    // we could extract the type from sema
    // so we can continue the code gen of this file
    // without having the end result

    // Build function type
    // FunctionType *fooFuncType = FunctionType::get(builder.getInt32Ty(), false);
    // Function *fooFunc = Function::Create(fooFuncType, Function::ExternalLinkage, "foo", module);

    // Declare an external global variable: @my_global
    // Type *globalType = builder.getInt32Ty(); // Assuming the type of the global is i32
    // GlobalVariable *myGlobal = new GlobalVariable(module, globalType, false,
    // GlobalValue::ExternalLinkage, nullptr, "my_global");

    return StmtRet();
}

StmtRet LLVMGen::importfrom(ImportFrom_t* n, int depth) {
    //
    return StmtRet();
}

StmtRet LLVMGen::match(Match_t* n, int depth) {
    //
    llvm::Value* val = exec(n->subject, depth).value();

    for (auto const& kase: n->cases) {
        // Generate the pattern matching conditions
        // we should
        exec(kase.pattern, depth);        //
        exec(kase.guard.value(), depth);  // <=

        // exec(kase.body, depth);
    }

    return StmtRet();
}

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
