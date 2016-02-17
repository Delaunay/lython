#include "Lexer.h"
#include "../fmt.h"

namespace lython{

std::ostream& Lexer::debug_print(std::ostream& out){

    Token t = next_token();
    int k = 1;
    do{
        out << to_string(k, 4) << "  ";
        t->debug_print(out) << std::endl;
        k += 1;
        t = next_token();
    }while((*this));

    out << to_string(k, 4) << "  ";
    t->debug_print(out) << std::endl;    // eof

    return out;
}
// print out tokens as they were inputed
std::ostream& Lexer::print(std::ostream& out){

    Token t = next_token();
    do{
        t->print(out);
        t = next_token();
    }while((*this));

    // send eof for reset
    t->print(out);

    return out;
}

}
