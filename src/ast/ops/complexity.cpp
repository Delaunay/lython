
#include "treewalk.h"

#include "utilities/magic.h"

namespace lython {

struct ComplexityVisitorTrait {
    using Trace   = std::false_type;
    using StmtRet = int;
    using ExprRet = int;
    using ModRet  = int;
    using PatRet  = int;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

class Complexity: public TreeWalk<Complexity, true, ComplexityVisitorTrait> {
    using Super = TreeWalk<Complexity, true, ComplexityVisitorTrait>;

    int edges = 0;
    int nodes = 0;
    int connected = 0;

    template<typename A>
    int complexity(int& acc, A* original, int depth) {
        acc = std::max(acc, exec(original[0], depth))
        return acc;
    }


    // McCabe showed that the cyclomatic complexity of a structured program with only one entry point and one exit point
    // is equal to the number of decision points ("if" statements or conditional loops) 
    // contained in that program plus one

    // Expression
    // ----------

    int boolop(BoolOp_t* n, int depth) {
        // in the context of a if cond, the complexity is dependent on the number of conditions
        return len(n->values);
    }

    // Statement
    // ---------

    int ifstmt(If_t* n, int depth) {
        int acc = 0;
        nodes += 1;
        edges += 2;

        complexity(acc, &n->test, depth);

        for (int i = 0; i < n->body.size(); i++) {
            complexity(acc, n->body[i], depth);
        }
        for (int i = 0; i < n->orelse.size(); i++) {
            complexity(acc, n->orelse[i], depth);
        }
        return acc + 1 + int(!n->orelse.empty());
    }

    int forstmt(For_t* n, int depth)  {

    }

    int trystmt(Try_t* n, int depth) {
        
    }

};

}