#include "Expression.h"
#include "../Parser/Parser.h"

#if LLVM_CODEGEN

#define LLVM_CHAPTER7 1

#define NEW_VAL(x)  g.gc.new_gen_function(x)

// ShortCut
// because there are llvm::Function and AST::Function
typedef llvm::Function LFunction;
typedef llvm::FunctionType LFunctionType;
typedef llvm::BasicBlock BasicBlock;
typedef llvm::Type Type;
typedef llvm::Value Value;

#define global_context() llvm::getGlobalContext()

namespace lython
{
namespace AbstractSyntaxTree{

    typedef llvm::Function::arg_iterator ArgIte;

    /*!
     * \brief Prototype::code_gen
     * \param g
     * \return
     */
    llvm::Function* Prototype::code_gen(Generator& g)
    {
        // Make the function type:  double(double,double) etc.
        std::vector<Type*> doubles(args.size(), Type::getDoubleTy(global_context()));

        LFunctionType *FT = LFunctionType::get(Type::getDoubleTy(global_context()),
                                              doubles, false);

        LFunction *F = LFunction::Create(FT,
                            LFunction::ExternalLinkage, name, g.module);

        // If F conflicted, there was already something named 'Name'.  If it has a
        // body, don't allow redefinition or reextern.
        if (F->getName() != name)
        {
            // Delete the one we just made and get the existing one.
            F->eraseFromParent();
            F = g.module->getFunction(name);

            // If F already has a body, reject this.
            if (!F->empty())
            {
                error<LFunction>("redefinition of function");
                return 0;
            }

            // If F took a different number of args, reject.
            if (F->arg_size() != args.size())
            {
                error<LFunction>("redefinition of function"
                                      " with different # args");
                return 0;
            }
        }

        // Set names for all arguments.
        unsigned i = 0;

        for (ArgIte ai = F->arg_begin(); i != args.size(); ++ai, ++i)
        {
            ai->setName(args[i]);

        #if !LLVM_CHAPTER7
            // Add arguments to variable symbol table.
            g.variables[args[i]] = ai;
        #endif
        }

        return F;
    }
/*!
 * \brief Function::code_gen
 * \param g
 * \return
 */
LFunction* Function::code_gen(Generator& g)
{
    g.variables.clear();

    LFunction *function = prototype->code_gen(g);

    if (function == 0)
        return 0;

    // USer defined operators
    if (prototype->is_binary())
        g.operators[prototype->operator_name()] = prototype->precedence();

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(global_context(), "entry", function);

    g.builder.SetInsertPoint(BB);

#if LLVM_CHAPTER7
    prototype->create_argument_allocas(g, function);
#endif

    Value *r = body->code_gen(g);

    if (r)
    {
        // Finish off the function.
        g.builder.CreateRet(r);

        // Validate the generated code, checking for consistency.
        verifyFunction(*function);

    #if LLVM_JIT
        g.fpm.run(*function);
    #endif

        return function;
    }

    // Error reading body, remove function.
    function->eraseFromParent();

    // WHAT DOES THIS THING DO delete the operator if it is a binary op
    // this piece of code appears from nowhere
    if (prototype->is_binary())
        g.operators.m().erase(prototype->operator_name());

    return 0;
}

/*!
 * \brief CallExpression::code_gen
 * \param g
 * \return
 */
Value* CallExpression::code_gen(Generator& g)
{
    // Look up the name in the global module table.
    LFunction *CalleeF = g.module->getFunction(callee);

    if (CalleeF == 0)
        return error<Value>("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != args.size())
        return error<Value>("Incorrect # arguments passed");

    std::vector<Value*> ArgsV;

    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        ArgsV.push_back(args[i]->code_gen(g));

        if (ArgsV.back() == 0)
            return 0;
    }

    return NEW_VAL(g.builder.CreateCall(CalleeF, ArgsV, "calltmp"));
}

/*!
 * \brief BinaryExpression::code_gen
 * \param g
 * \return
 */
Value* BinaryExpression::code_gen(Generator& g)
{
    // Special case '=' because we don't want to emit the LHS as an expression.
    if (op == '=')
    {
        // Assignment requires the LHS to be an identifier.
        AST::VariableExpression *LHSE = dynamic_cast<AST::VariableExpression*>(lhs);

        if (!LHSE)
            return error<Value>("destination of '=' must be a variable");

        // Codegen the RHS.
        Value *val = rhs->code_gen(g);

        if (val == 0)
            return 0;

        // Look up the name.
        Value *variable = g.variables[LHSE->name];

        if (variable == 0)
            return error<Value>("Unknown variable name");

        g.builder.CreateStore(val, variable);
        return val;
    }

    Value *L = lhs->code_gen(g);
    Value *R = rhs->code_gen(g);

    if (L == 0 || R == 0)
        return 0;

    switch (op)
    {
        case '+':
            return g.builder.CreateFAdd(L, R, "addtmp");

        case '-':
            return g.builder.CreateFSub(L, R, "subtmp");

        case '*':
            return g.builder.CreateFMul(L, R, "multmp");

        case '<':
            L = g.builder.CreateFCmpULT(L, R, "cmptmp");

            // Convert bool 0/1 to double 0.0 or 1.0
            return g.builder.CreateUIToFP(L, Type::getDoubleTy(global_context()),
                                    "booltmp");
        default:
            break;
            //return error<Value>("invalid binary operator");
    }

    // User Defined operator
    LFunction* f = g.module->getFunction(std::string("binary") + op);
    assert(f && "binary operator not found");

    Value* ops[2] = {L, R};

    return NEW_VAL(g.builder.CreateCall(f, ops, "binop"));
}

/*!
 * \brief VariableExpression::code_gen
 * \param g
 * \return
 */
Value* VariableExpression::code_gen(Generator& g)
{
    Value *v = g.variables[name];

    if (v == 0)
        error<Value>("Unknown variable name");

    // Chapter 7 Mutable Variable
    return g.builder.CreateLoad(v, name.c_str());
}

Value* IfExpression::code_gen(Generator& g)
{
    Value *condv = cond->code_gen(g);

    if (condv == 0)
        return 0;

      // Convert condition to a bool by comparing equal to 0.0.
      condv = g.builder.CreateFCmpONE(
          condv, llvm::ConstantFP::get(global_context(),
                                       llvm::APFloat(0.0)), "ifcond");

      LFunction *function = g.builder.GetInsertBlock()->getParent();

      // Create blocks for the then and else cases.  Insert the 'then' block at the
      // end of the function.
      BasicBlock *thenblock  = BasicBlock::Create(global_context(), "then", function);
      BasicBlock *elseblock  = BasicBlock::Create(global_context(), "else");
      BasicBlock *mergeblock = BasicBlock::Create(global_context(), "ifcont");

      g.builder.CreateCondBr(condv, thenblock, elseblock);

      // Emit then value.
      g.builder.SetInsertPoint(thenblock);

      Value *thenv = then->code_gen(g);

      if (thenv == 0)
        return 0;

      g.builder.CreateBr(mergeblock);

      // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
      thenblock = g.builder.GetInsertBlock();

      // Emit else block.
      function->getBasicBlockList().push_back(elseblock);
      g.builder.SetInsertPoint(elseblock);

      Value *elsev = els->code_gen(g);
      if (elsev == 0)
        return 0;

      g.builder.CreateBr(mergeblock);
      // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
      elseblock = g.builder.GetInsertBlock();

      // Emit merge block.
      function->getBasicBlockList().push_back(mergeblock);
      g.builder.SetInsertPoint(mergeblock);

      llvm::PHINode *PN =
          g.builder.CreatePHI(Type::getDoubleTy(global_context()), 2, "iftmp");

      PN->addIncoming(thenv, thenblock);
      PN->addIncoming(elsev, elseblock);

      return NEW_VAL(PN);
}

// Output this as:
//   ...
//   start = startexpr
//   goto loop
// loop:
//   variable = phi [start, loopheader], [nextvariable, loopend]
//   ...
//   bodyexpr
//   ...
// loopend:
//   step = stepexpr
//   nextvariable = variable + step
//   endcond = endexpr
//   br endcond, loop, endloop
// outloop:

Value* ForExpression::code_gen(Generator& g)
{
    // Make the new basic block for the loop header, inserting after current block.
    LFunction *function = g.builder.GetInsertBlock()->getParent();

#if LLVM_CHAPTER7
    llvm::AllocaInst* aalloca = Generator::create_entry_block_alloca(function, var);
#endif

    // Emit the start code first, without 'variable' in scope.
    Value *start_val = start->code_gen(g);

    if (start_val == 0)
        return 0;

#if LLVM_CHAPTER7
    // Store the value into the alloca.
    g.builder.CreateStore(start_val, aalloca);
#else
    BasicBlock *preheader_block = g.builder.GetInsertBlock();
#endif

    BasicBlock *loop_block =
    BasicBlock::Create(global_context(), "loop", function);

    // Insert an explicit fall through from the current block to the LoopBB.
    g.builder.CreateBr(loop_block);

    // Start insertion in LoopBB.
    g.builder.SetInsertPoint(loop_block);

#if LLVM_CHAPTER7
    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    llvm::AllocaInst *old_val = g.variables[var];

    g.variables[var] = aalloca;
#else
    // Start the PHI node with an entry for Start.
    llvm::PHINode *variable = g.builder.CreatePHI(Type::getDoubleTy(global_context()),
                                        2, var.c_str());

    variable->addIncoming(start_val, preheader_block);

    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.

    Value *old_val = g.variables[var];
    g.variables[var] = variable;
#endif

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    if (body->code_gen(g) == 0)
        return 0;

    // Emit the step value.
    Value *step_val;

    if (step)
    {
        step_val = step->code_gen(g);

        if (step_val == 0)
            return 0;
    }
    else
    {
        // If not specified, use 1.0.
        step_val = llvm::ConstantFP::get(global_context(), llvm::APFloat(1.0));
    }

    // Compute the end condition.
    Value *end_cond = end->code_gen(g);

    if (end_cond == 0)
        return end_cond;

#if LLVM_CHAPTER7
    Value *cur_var = g.builder.CreateLoad(aalloca, var.c_str());
    Value *next_var = g.builder.CreateFAdd(cur_var, step_val, "nextvar");
    g.builder.CreateStore(next_var, aalloca);
#else
    Value *next_var = g.builder.CreateFAdd(variable, step_val, "nextvar");

    // Create the "after loop" block and insert it.
    BasicBlock *loop_end_block = g.builder.GetInsertBlock();
#endif

    // Convert condition to a bool by comparing equal to 0.0.
    end_cond = g.builder.CreateFCmpONE(
    end_cond, llvm::ConstantFP::get(global_context(), llvm::APFloat(0.0)), "loopcond");

    BasicBlock *after_block =
    BasicBlock::Create(global_context(), "afterloop", function);

    // Insert the conditional branch into the end of LoopEndBB.
    g.builder.CreateCondBr(end_cond, loop_block, after_block);

    // Any new code will be inserted in AfterBB.
    g.builder.SetInsertPoint(after_block);

#if !LLVM_CHAPTER7
    // Add a new entry to the PHI node for the backedge.
    variable->addIncoming(next_var, loop_end_block);
#endif

    // Restore the unshadowed variable.
    if (old_val)
        g.variables[var] = old_val;
    else
        g.variables.erase(var);

    // for expr always returns 0.0.
    return NEW_VAL(llvm::Constant::getNullValue(Type::getDoubleTy(global_context())));//*/
}

Value* UnaryExpression::code_gen(Generator &g)
{
    Value *op_val = operand->code_gen(g);

    if (op_val == 0)
        return 0;

    LFunction *f = g.module->getFunction(std::string("unary") + opcode);

    if (f == 0)
        return error<Value>("Unknown unary operator");

    return NEW_VAL(g.builder.CreateCall(f, op_val, "unop"));
}

void Prototype::create_argument_allocas(Generator &g, llvm::Function* f)
{
    llvm::Function::arg_iterator ai = f->arg_begin();

    for (unsigned i = 0, e = args.size(); i != e; ++i, ++ai)
    {
        // Create an alloca for this variable.
        llvm::AllocaInst *aalloca = Generator::create_entry_block_alloca(f, args[i]);

        // Store the initial value into the alloca.
        g.builder.CreateStore(ai, aalloca);

        // Add arguments to variable symbol table.
        g.variables[args[i]] = aalloca;
    }
}

llvm::Value* MutableVariableExpression::code_gen(Generator &g)
{
    std::vector<llvm::AllocaInst *> oldbind;

    llvm::Function *function = g.builder.GetInsertBlock()->getParent();

    // Register all variables and emit their initializer.
    for (unsigned i = 0, e = var_names.size(); i != e; ++i)
    {
        const std::string &var_name = var_names[i].first;
        AST::Expression *init = var_names[i].second;

        // Emit the initializer before adding the variable to scope, this prevents
        // the initializer from referencing the variable itself, and permits stuff
        // like this:
        //  var a = 1 in
        //    var a = a in ...   # refers to outer 'a'.
        Value *initval;

        if (init)
        {
            initval = init->code_gen(g);
            if (initval == 0)
                return 0;
        }
        else
        {
            // If not specified, use 0.0.
            initval = llvm::ConstantFP::get(global_context(), llvm::APFloat(0.0));
        }

        llvm::AllocaInst *Alloca = Generator::create_entry_block_alloca(function, var_name);
        g.builder.CreateStore(initval, Alloca);

        // Remember the old variable binding so that we can restore the binding when
        // we unrecurse.
        oldbind.push_back(g.variables[var_name]);

        // Remember this binding.
        g.variables[var_name] = Alloca;
    }

    // Codegen the body, now that all vars are in scope.
    Value *bodyval = body->code_gen(g);

    if (bodyval == 0)
        return 0;

    // Pop all our variables from scope.
    for (unsigned i = 0, e = var_names.size(); i != e; ++i)
        g.variables[var_names[i].first] = oldbind[i];

    // Return the body computation.
    return bodyval;
}

}
}
#endif
