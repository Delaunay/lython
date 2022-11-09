#pragma once

#include "lexer/token.h"

namespace lython {

// NB: it is important to be able to generate the code as is
// from the tokens since when handling with bad code in an interactive environemtn
// we will save the WIP code as a list of tokens
// could be used for code formatting
class Unlex {
    public:
    std::ostream& format(std::ostream& out, Array<Token> const& tokens);
    std::ostream& format(std::ostream& out, Token const& token, int indent = 0);

    void reset();

    bool stop_on_newline = false;

    private:
    bool  should_stop  = false;
    int32 indent_level = 0;
    bool  emptyline    = true;  // To generate indent when needed
    bool  open_parens  = false;
    bool  prev_is_op   = false;
};

}  // namespace lython