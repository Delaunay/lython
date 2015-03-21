#ifndef LYTHON_PARSER_OBJECTMANAGER_HEADER
#define LYTHON_PARSER_OBJECTMANAGER_HEADER

#include "../AbstractSyntaxTree/Expression.h"

// #include <vector>
// using namespace std;

namespace lython{

/*  Stupid Garbage Colector, save adresses for deletion
 *
 *  Valgrind Report:
 *
 * LEAK SUMMARY:
 *      ==10203==    definitely lost: 0 bytes in 0 blocks
 *      ==10203==    indirectly lost: 0 bytes in 0 blocks
 *      ==10203==      possibly lost: 0 bytes in 0 blocks
 *      ==10203==    still reachable: 9,254 bytes in 30 blocks
 *      ==10203==         suppressed: 0 bytes in 0 blocks
 *
 * does they matter ?
 */
class ObjectManager
{
    public:
        ObjectManager()
        {}

        ~ObjectManager();

        AST::Expression* new_expr(AST::Expression* expr);
        AST::Prototype* new_prototype(AST::Prototype* expr);
        AST::Function* new_function(AST::Function* expr);
    #if LLVM_CODEGEN
        llvm::Function* new_gen_function(llvm::Function* expr);
    #endif

     vector<AST::Expression*> expression;
     vector<AST::Prototype*>  prototype;
     vector<AST::Function*>   function;

#if LLVM_CODEGEN
     vector<llvm::Function*>  generated;
#endif
};

}
#endif
