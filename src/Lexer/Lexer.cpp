#include "Lexer.h"
#include "../fmt.h"

namespace lython{

std::ostream& Lexer::debug_print(std::ostream& out){

    Token t = next_token();
    int k = 1;
    do{
        out << to_string(k, 4) << "  ";
        t.debug_print(out) << std::endl;
        k += 1;
    }while((t = next_token()));

    out << to_string(k, 4) << "  ";
    t.debug_print(out) << std::endl;    // eof

    return out;
}
// print out tokens as they were inputed
std::ostream& Lexer::print(std::ostream& out){

    Token t = next_token();
    do{
        t.print(out);
    }while((t = next_token()));

    // send eof for reset
    t.print(out);

    return out;
}

}
