#include "parser/parser.h"

namespace lython {

Expression Parser::parse_loop(Module& m, std::size_t depth){
    EAT(tok_while);
    EAT(tok_for);
    consume_block(depth);
    return Expression();
}

}
