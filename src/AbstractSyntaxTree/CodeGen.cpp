#include "Expression.h"
#include "../Parser/Parser.h"

#if LLVM_CODEGEN

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
        std::vector<llvm::Type*> doubles(args.size(),
                                   llvm::Type::getDoubleTy(llvm::getGlobalContext()));

        llvm::FunctionType *FT = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                    doubles, false);

        llvm::Function *F = llvm::Function::Create(FT,
                            llvm::Function::ExternalLinkage, name, g.module);

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
                error<llvm::Function>("redefinition of function");
                return 0;
            }

            // If F took a different number of args, reject.
            if (F->arg_size() != args.size())
            {
                error<llvm::Function>("redefinition of function"
                                      " with different # args");
                return 0;
            }
        }

        // Set names for all arguments.
        unsigned i = 0;

        for (ArgIte ai = F->arg_begin(); i != args.size(); ++ai, ++i)
        {
            ai->setName(args[i]);

            // Add arguments to variable symbol table.
            g.variables[args[i]] = ai;
        }

        return F;
    }
/*!
 * \brief Function::code_gen
 * \param g
 * \return
 */
llvm::Function* Function::code_gen(Generator& g)
{
    g.variables.clear();

    llvm::Function *function = prototype->code_gen(g);

    if (function == 0)
        return 0;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvm::getGlobalContext(),
                                        "entry", function);

    g.builder.SetInsertPoint(BB);

    llvm::Value *r = body->code_gen(g);

    if (r)
    {
        // Finish off the function.
        g.builder.CreateRet(r);

        // Validate the generated code, checking for consistency.
        verifyFunction(*function);

        return function;
    }

    // Error reading body, remove function.
    function->eraseFromParent();
    return 0;
}

/*!
 * \brief CallExpression::code_gen
 * \param g
 * \return
 */
llvm::Value* CallExpression::code_gen(Generator& g)
{
    // Look up the name in the global module table.
    llvm::Function *CalleeF = g.module->getFunction(callee);

    if (CalleeF == 0)
        return error<llvm::Value>("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != args.size())
        return error<llvm::Value>("Incorrect # arguments passed");

    std::vector<llvm::Value*> ArgsV;

    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        ArgsV.push_back(args[i]->code_gen(g));

        if (ArgsV.back() == 0)
            return 0;
    }

    return g.builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

/*!
 * \brief BinaryExpression::code_gen
 * \param g
 * \return
 */
llvm::Value* BinaryExpression::code_gen(Generator& g)
{
    llvm::Value *L = lhs->code_gen(g);
    llvm::Value *R = rhs->code_gen(g);

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
            return g.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                                    "booltmp");
        default:
            return error<llvm::Value>("invalid binary operator");
    }
}

/*!
 * \brief VariableExpression::code_gen
 * \param g
 * \return
 */
llvm::Value* VariableExpression::code_gen(Generator& g)
{
    llvm::Value *v = g.variables[name];

    if (v)
        return v;
    else
        error<llvm::Value>("Unknown variable name");
}



llvm::Value* IfExpression::code_gen(Generator& g)
{
    llvm::Value *condv = cond->code_gen(g);
      if (condv == 0)
        return 0;

      // Convert condition to a bool by comparing equal to 0.0.
      condv = g.builder.CreateFCmpONE(
          condv, llvm::ConstantFP::get(llvm::getGlobalContext(),
                                       llvm::APFloat(0.0)), "ifcond");

      llvm::Function *function = g.builder.GetInsertBlock()->getParent();

      // Create blocks for the then and else cases.  Insert the 'then' block at the
      // end of the function.
      llvm::BasicBlock *thenblock  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", function);
      llvm::BasicBlock *elseblock  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
      llvm::BasicBlock *mergeblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifcont");

      g.builder.CreateCondBr(condv, thenblock, elseblock);

      // Emit then value.
      g.builder.SetInsertPoint(thenblock);

      llvm::Value *thenv = then->code_gen(g);

      if (thenv == 0)
        return 0;

      g.builder.CreateBr(mergeblock);

      // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
      thenblock = g.builder.GetInsertBlock();

      // Emit else block.
      function->getBasicBlockList().push_back(elseblock);
      g.builder.SetInsertPoint(elseblock);

      llvm::Value *elsev = els->code_gen(g);
      if (elsev == 0)
        return 0;

      g.builder.CreateBr(mergeblock);
      // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
      elseblock = g.builder.GetInsertBlock();

      // Emit merge block.
      function->getBasicBlockList().push_back(mergeblock);
      g.builder.SetInsertPoint(mergeblock);

      llvm::PHINode *PN =
          g.builder.CreatePHI(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 2, "iftmp");

      PN->addIncoming(thenv, thenblock);
      PN->addIncoming(elsev, elseblock);

      return PN;
}


llvm::Value* ForExpression::code_gen(Generator& g)
{
    /*
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

    // Emit the start code first, without 'variable' in scope.
    Value *StartVal = Start->Codegen();
    if (StartVal == 0)
        return 0;

    // Make the new basic block for the loop header, inserting after current
    // block.
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *PreheaderBB = Builder.GetInsertBlock();
    BasicBlock *LoopBB =
      BasicBlock::Create(getGlobalContext(), "loop", TheFunction);

    // Insert an explicit fall through from the current block to the LoopBB.
    Builder.CreateBr(LoopBB);

    // Start insertion in LoopBB.
    Builder.SetInsertPoint(LoopBB);

    // Start the PHI node with an entry for Start.
    PHINode *Variable = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()),
                                        2, VarName.c_str());
    Variable->addIncoming(StartVal, PreheaderBB);

    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    Value *OldVal = NamedValues[VarName];
    NamedValues[VarName] = Variable;

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    if (Body->Codegen() == 0)
        return 0;

    // Emit the step value.
    Value *StepVal;
    if (Step)
    {
        StepVal = Step->Codegen();

        if (StepVal == 0)
            return 0;
    }
    else
    {
        // If not specified, use 1.0.
        StepVal = ConstantFP::get(getGlobalContext(), APFloat(1.0));
    }

    Value *NextVar = Builder.CreateFAdd(Variable, StepVal, "nextvar");

    // Compute the end condition.
    Value *EndCond = End->Codegen();
    if (EndCond == 0)
        return EndCond;

    // Convert condition to a bool by comparing equal to 0.0.
    EndCond = Builder.CreateFCmpONE(
      EndCond, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *LoopEndBB = Builder.GetInsertBlock();
    BasicBlock *AfterBB =
      BasicBlock::Create(getGlobalContext(), "afterloop", TheFunction);

    // Insert the conditional branch into the end of LoopEndBB.
    Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

    // Any new code will be inserted in AfterBB.
    Builder.SetInsertPoint(AfterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(NextVar, LoopEndBB);

    // Restore the unshadowed variable.
    if (OldVal)
        NamedValues[VarName] = OldVal;
    else
        NamedValues.erase(VarName);

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));*/
}

}
}
#endif
