#include "Grammar.h"
#include "Grammar.def"

namespace lython{
Grammar& default_grammar(){

    static Grammar grm = {
    #define X(op, ll, lr) {std::string(op), Precedence(ll, lr)},
        LYTHON_DEFAULT_GRAMMAR
    #undef X
    };

    return grm;
}
}
