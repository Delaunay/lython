#if WITH_LLVM && WITH_LLVM_CODEGEN 

// include
#include "codegen/llvm/llvm_gen.h"

// Kiwi
#include "ast/magic.h"
#include "builtin/operators.h"
#include "utilities/guard.h"
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

    Value*                    callee   = exec(n->func, depth);
    Function*                 function = dyn_cast_or_null<Function>(callee);
    llvm::FunctionType const* ftype    = nullptr;

    if (function == nullptr) {
        kwerror("Function is not callable");
        return nullptr;
    }
    ftype = function->getFunctionType();

    Array<Value*> args;
    for (size_t i = 0, end = n->args.size(); i != end; ++i) {
        ExprRet argvalue = exec(n->args[i], depth);
        args.push_back(argvalue);

        if (argvalue == nullptr) {
            kwerror("Could not generate function call");
            return nullptr;
        }
    }

    kwassert(function->arg_size() == args.size(), "Argument should match");
    for (size_t i = 0; i < args.size(); i++) {
        auto* arg_type = ftype->getParamType((unsigned int)i);
        auto* val_type = args[i]->getType();

        if (arg_type != val_type) {
            kwerror("Type mistmatch expected {} got {}", llvmstr(arg_type), llvmstr(val_type));
        }
    }

    return builder->CreateCall(function, args, "calltmp");
    // return builder->CreateCall(callee, args, "calltmp");
}

using BuiltinBinaryOperators = Dict<String, std::function<Value*(IRBuilder<>*, Value*, Value*)>>;

#define LLMV_OPERATORS(OP) \
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
    // return binary_operator(builder.get(), n->left, n->right, 0, depth);
    
    Value* left  = exec(n->left, depth);
    Value* right = exec(n->right, depth);

    if (left == nullptr || right == nullptr) {
        kwerror("Could not generate binary operator");
        return nullptr;
    }

    switch(n->op) {
        case BinaryOperator::Add: {
            return builder->CreateFAdd(left, right, "addtmp");
            return builder->CreateAdd(left, right, "addtmp");
        }
        case BinaryOperator::Sub:       
            return builder->CreateFSub(left, right, "subtmp");
            return builder->CreateSub(left, right, "subtmp");
        
        case BinaryOperator::Mult:      
            return builder->CreateFMul(left, right, "multtmp");
            return builder->CreateMul(left, right, "multtmp");
        
        case BinaryOperator::Div:       
            return builder->CreateFDiv(left, right, "divtmp");
            return builder->CreateSDiv(left, right, "divtmp");
            return builder->CreateUDiv(left, right, "divtmp");

        case BinaryOperator::Mod: {      
            // Function *powFunc = Intrinsic::getDeclaration(llmodule.get(), Intrinsic::mo, {builder->getDoubleTy()});
            return builder->CreateSRem(left, right);
            return builder->CreateFRem(left, right);
            return builder->CreateURem(left, right);
        }
        case BinaryOperator::Pow: {      
            Function *powFunc = Intrinsic::getDeclaration(llmodule.get(), Intrinsic::pow, {builder->getDoubleTy()});
            return builder->CreateCall(powFunc, {left, right});;
        }
        case BinaryOperator::LShift:    return builder->CreateLShr(left, right, "addtmp");
        case BinaryOperator::RShift:    return builder->CreateShl(left, right, "addtmp");
        case BinaryOperator::BitOr:     return builder->CreateOr(left, right, "bitortmp");
        case BinaryOperator::BitXor:    return builder->CreateXor(left, right, "bitxortmp");
        case BinaryOperator::BitAnd:    return builder->CreateAnd(left, right, "bitandtmp");

        case BinaryOperator::FloorDiv: {      
            return builder->CreateSDiv(left, right, "sdivtmp");    
            // return builder->CreateUDiv(left, right, "");
        }
        case BinaryOperator::MatMult:   return nullptr;
        case BinaryOperator::EltDiv:    return nullptr;
        case BinaryOperator::EltMult:   return nullptr;
    
    }

    kwerror("binary operator not handled");
    return nullptr;
}
ExprRet LLVMGen::boolop(BoolOp_t* n, int depth) { 

    Array<Value*> values(n->values.size());

    for(int i = 0; i < n->values.size(); i++) {
        values[i] = exec(n->values[i], depth);
    }

    auto fun = [this, n](Value* a, Value* b) -> Value* {
        switch (n->op) {
            case BoolOperator::Or:    
                return builder->CreateOr(a, b, "ortmp");
            case BoolOperator::And:    
                return builder->CreateAnd(a, b, "andtmp");
        }
    };

    // does this even matter ?
    // log2(op)
    while (values.size() > 2) {
        int count = values.size() / 2;
        int extra = values.size() % 2;
        Array<Value*> next(count + extra);

        for(int i = 0; i < count; i++) {
            next[i] = fun(values[i * 2], values[i * 2 + 1]);
        }
        if (extra > 0) {
            next[count + extra] = values[values.size() - 1];
        }
        values = next;
    }
    return values[0]; 
}

ExprRet LLVMGen::unaryop(UnaryOp_t* n, int depth) { 
    Value* arg = exec(n->operand, depth);

    switch(n->op) {
        case UnaryOperator::Invert: 
            return builder->CreateXor(arg, ConstantInt::get(builder->getInt32Ty(), -1));
        case UnaryOperator::Not:
            // arg xor 1
            return builder->CreateXor(arg, ConstantInt::get(builder->getInt1Ty(), 1));
        case UnaryOperator::UAdd:   
            return arg;
        case UnaryOperator::USub:   
            return builder->CreateSub(ConstantInt::get(builder->getInt32Ty(), 0), arg, "subtmp");
    }
    return nullptr; 
}
ExprRet LLVMGen::compare(Compare_t* n, int depth) { 
    
    auto fun = [this](CmpOperator op, Value* lhs, Value* rhs) -> Value* {
        switch (op) {
            case CmpOperator::Eq:    return builder->CreateFCmp(CmpInst::FCMP_OEQ, lhs, rhs, "");
            case CmpOperator::NotEq: return builder->CreateFCmp(CmpInst::FCMP_UNE, lhs, rhs, "");
            case CmpOperator::Lt:    return builder->CreateFCmpOLT(lhs, rhs, "");
            case CmpOperator::LtE:   return builder->CreateFCmpOLE(lhs, rhs, "");
            case CmpOperator::Gt:    return builder->CreateFCmpOGT(lhs, rhs, "");
            case CmpOperator::GtE:   return builder->CreateFCmpOGE(lhs, rhs, "");
            case CmpOperator::Is:    return nullptr;
            case CmpOperator::IsNot: return nullptr;
            case CmpOperator::In:    return nullptr;
            case CmpOperator::NotIn: return nullptr;
        }
        // this->builder->CreateFCmpOLT()
    };

    Value* left = exec(n->left, depth);
    Array<Value*> comparisons;

    for(int i = 0; i < n->ops.size(); i++) {
        Value* right = exec(n->comparators[i], depth);
        comparisons.push_back(fun(n->ops[i], left, right));
        left = right;
    }

    Value* prev = comparisons[0];
    for(int i = 1; i < comparisons.size(); i++) {
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
ExprRet LLVMGen::exported(Exported* n, int depth) {
    return nullptr;
}   
ExprRet LLVMGen::lambda(Lambda_t* n, int depth) { 
    Function *lambdaFunc = Function::Create(
        nullptr, // lambdaFuncType, 
        Function::ExternalLinkage, 
        "", 
        llmodule.get()
    );

    return ExprRet(); 
}
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

    fundef->insert(fundef->end(), elxpr);
    // fundef->getBasicBlockList().push_back(elxpr);

    // orelse
    builder->SetInsertPoint(elxpr);
    Value* else_value = exec(n->orelse, depth);
    builder->CreateBr(merged);
    // -----

    elxpr = builder->GetInsertBlock();

    // fundef->getBasicBlockList().push_back(merged);
    fundef->insert(fundef->end(), merged);

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
ExprRet LLVMGen::placeholder(Placeholder_t* n, int depth) { return ExprRet(); }
ExprRet LLVMGen::constant(Constant_t* n, int depth) {

    ConstantValue const& val = n->value;
    using Ty                 = double;

    // clang-format off
    switch (n->value.get_kind()) {
    #if 1
    case ConstantValue::Ti8:  return ConstantFP::get(*context, APFloat((Ty)val.get<int8>   ()));
    case ConstantValue::Ti16: return ConstantFP::get(*context, APFloat((Ty)val.get<int16>  ()));
    case ConstantValue::Ti32: return ConstantFP::get(*context, APFloat((Ty)val.get<int32>  ()));
    case ConstantValue::Ti64: return ConstantFP::get(*context, APFloat((Ty)val.get<int64>  ()));
    case ConstantValue::Tu8:  return ConstantFP::get(*context, APFloat((Ty)val.get<uint8>  ()));
    case ConstantValue::Tu16: return ConstantFP::get(*context, APFloat((Ty)val.get<uint16> ()));
    case ConstantValue::Tu32: return ConstantFP::get(*context, APFloat((Ty)val.get<uint32> ()));
    case ConstantValue::Tu64: return ConstantFP::get(*context, APFloat((Ty)val.get<uint64> ()));
    case ConstantValue::Tf32: return ConstantFP::get(*context, APFloat((Ty)val.get<float32>()));
    case ConstantValue::Tf64: return ConstantFP::get(*context, APFloat((Ty)val.get<float64>()));
    case ConstantValue::TBool:return ConstantFP::get(*context, APFloat((Ty)val.get<int8>   ()));
    #else
    case ConstantValue::Ti8:  return ConstantInt::get(*context, APInt(8,  (int8)   val.get<int8>   (), true));
    case ConstantValue::Ti16: return ConstantInt::get(*context, APInt(16, (int16)  val.get<int16>  (), true));
    case ConstantValue::Ti32: return ConstantInt::get(*context, APInt(32, (int32)  val.get<int32>  (), true));
    case ConstantValue::Ti64: return ConstantInt::get(*context, APInt(64, (int64)  val.get<int64>  (), true));
    case ConstantValue::Tu8:  return ConstantInt::get(*context, APInt(8,  (uint8)  val.get<uint8>  ()));
    case ConstantValue::Tu16: return ConstantInt::get(*context, APInt(16, (uint16) val.get<uint16> ()));
    case ConstantValue::Tu32: return ConstantInt::get(*context, APInt(32, (uint32) val.get<uint32> ()));
    case ConstantValue::Tu64: return ConstantInt::get(*context, APInt(64, (uint64) val.get<uint64> ()));
    case ConstantValue::Tf32: return ConstantFP ::get(*context, APFloat(  (float32)val.get<float32>()));
    case ConstantValue::Tf64: return ConstantFP ::get(*context, APFloat(  (float64)val.get<float64>()));
    case ConstantValue::TBool:return ConstantInt::get(*context, APInt(8,  (int8)   val.get<int8>   ()));
    #endif
    }
    // clang-format on

    // TODO: object are a bit more complex
    // llvm::ConstantStruct::get();
    // llvm::ConstantArray

    return nullptr;
}
ExprRet LLVMGen::attribute(Attribute_t* n, int depth) { 
    // struct lookup
    return ExprRet(); 
}
ExprRet LLVMGen::subscript(Subscript_t* n, int depth) { 
    // slice
    return ExprRet(); 
}
ExprRet LLVMGen::starred(Starred_t* n, int depth) { 
    //
    return ExprRet(); 
}

ExprRet LLVMGen::name(Name_t* n, int depth) {

    if (n->ctx == ExprContext::Store) {
        Function*   fundef = builder->GetInsertBlock()->getParent();
        IRBuilder<> allocabuilder(&fundef->getEntryBlock(), fundef->getEntryBlock().begin());
        AllocaInst* variable  = allocabuilder.CreateAlloca(  //
            Type::getDoubleTy(*context),                    //
            nullptr,                                        //
            tostr(n->id)                                    //
        );
        index_to_index[n->id] = named_values.size();
        named_values.push_back(variable);
        return variable;
    }

    auto index = index_to_index[n->id];
    kwinfo("Size: {} varid: {} index: {}", named_values.size(), n->varid, index);
    Value* value = named_values[index];

    if (value == nullptr) {
        return nullptr;
    }

#if WITH_LLVM_DEBUG_SYMBOL
    KSDbgInfo.emitLocation(this);
#endif

    if (!value->getType()->getPointerElementType()->isSized()) {
        return value;
    }

    // Load the value.
    // return builder->CreateLoad(value, tostr(n->id));
    return builder->CreateLoad(value->getType()->getPointerElementType(), value, tostr(n->id));
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
    llvm::FunctionType* arrow = llvm::FunctionType::get(  //
        Type::getDoubleTy(*context),                      //
        arg_types,                                        //
        false                                             //
    );

    Function* fundef = Function::Create(  //
        arrow,                            //
        Function::ExternalLinkage,        //
        tostr(n->name),                   //
        llmodule.get()                    //
    );

    index_to_index[n->name] = named_values.size();
    named_values.push_back(fundef);

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

    unsigned                 i = 0;
    ArrayScope<llvm::Value*> scope(named_values);

    for (auto& arg: fundef->args()) {
        Identifier  argname_ref = n->args.args[i].arg;
        std::string argname     = tostr(argname_ref);
        arg.setName(argname);

        IRBuilder<> allocabuilder(&fundef->getEntryBlock(), fundef->getEntryBlock().begin());
        AllocaInst* arg_mem         = allocabuilder.CreateAlloca(  //
            Type::getDoubleTy(*context),                   //
            nullptr,                                       //
            arg.getName()                                  //
        );
        index_to_index[argname_ref] = named_values.size();
        named_values.push_back(arg_mem);

        // CreateEntryBlockAlloca(fundef, arg.getName());

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

    verifyFunction(*fundef);
    fun_optim->run(*fundef);

    return StmtRet();
}

llvm::Type* LLVMGen::builtin_type(StringRef name) {
    static Dict<StringRef, llvm::Type*> builtin_types = {
        {StringRef("i8"), IntegerType::get(*context, 8)},
        {StringRef("i16"), IntegerType::get(*context, 16)},
        {StringRef("i32"), IntegerType::get(*context, 32)},
        {StringRef("i64"), IntegerType::get(*context, 64)},

        {StringRef("u8"), IntegerType::get(*context, 8)},
        {StringRef("u16"), IntegerType::get(*context, 16)},
        {StringRef("u32"), IntegerType::get(*context, 32)},
        {StringRef("u64"), IntegerType::get(*context, 64)},

        {StringRef("f32"), Type::getFloatTy(*context)},
        {StringRef("f64"), Type::getDoubleTy(*context)},
    };
    return builtin_types[name];
}

llvm::Type* LLVMGen::retrieve_type(ExprNode* type, int depth) {
    kwinfo("{}", str(type));

    switch (type->kind) {
    case NodeKind::BuiltinType: return builtin_type(cast<BuiltinType>(type)->name);
    case NodeKind::Name: return builtin_type(cast<Name>(type)->id);
    }
    return nullptr;
}

StmtRet LLVMGen::classdef(ClassDef_t* n, int depth) {
    // Create the type definition
    Array<llvm::Type*> fields;
    llvm::Type*        int_type = llvm::Type::getInt32Ty(*context);

    for (ClassDef::Attr const& attr: n->attributes) {
        llvm::Type* field_type = retrieve_type(attr.type, depth);
        if (field_type == nullptr) {
            kwerror("Could not find type for {}", str(attr.type));
        }
        fields.push_back(field_type);
    }

    StructType* clstype = llvm::StructType::create(*context, fields, tostr(n->name));
    llmodule->getOrInsertGlobal(tostr(n->name), clstype);

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
    // fundef->getBasicBlockList().push_back(elxpr);
    fundef->insert(fundef->end(), elxpr);

    // orelse
    builder->SetInsertPoint(elxpr);
    for (auto* stmt: n->orelse) {
        exec(stmt, depth);
    }
    builder->CreateBr(merged);
    // -----

    elxpr = builder->GetInsertBlock();

    // fundef->getBasicBlockList().push_back(merged);
    fundef->insert(fundef->end(), merged);

    builder->SetInsertPoint(merged);

    return StmtRet();
}
StmtRet LLVMGen::with(With_t* n, int depth) { return StmtRet(); }
StmtRet LLVMGen::raise(Raise_t* n, int depth) {
    BasicBlock* current_block = builder->GetInsertBlock();
    Function* fundef = current_block->getParent();

    Function* raisefun = nullptr;
    // Function* raisefun = Intrinsic::getDeclaration(fundef->getParent(), Intrinsic::eh_throw);
    Value*    exception = nullptr;
    CallInst* raise     = builder->CreateCall(raisefun, {exception});
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
    Value* test = exec(n->test, depth);

    // BasicBlock* merged = BasicBlock::Create(*context, "ifcont");
    // Value* condcmp = builder->CreateFCmpONE(
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
    // GlobalVariable *myGlobal = new GlobalVariable(module, globalType, false, GlobalValue::ExternalLinkage, nullptr, "my_global");


    return StmtRet(); 
}

StmtRet LLVMGen::importfrom(ImportFrom_t* n, int depth) { 
    //
    return StmtRet(); 
}

StmtRet LLVMGen::match(Match_t* n, int depth) { 
    //
    Value* val = exec(n->subject, depth);

    for(auto const& kase: n->cases) {
        // Generate the pattern matching conditions
        // we should
        exec(kase.pattern, depth);          //
        exec(kase.guard.value(), depth);    // <= 

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
