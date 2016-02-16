#include "Prelexer.h"
#include "../fmt.h"

#define PRETOK_SIZE 20

namespace lython {

std::ostream& PreToken::debug_print(std::ostream& out){
    //static int indent_level = 0;

    if(type() == pretok_prestring){
        out << "    [l:" << to_string(line(), 4) << " c:"<< to_string(col(), 4) << "] "
            << align_right("pretok_prestring", PRETOK_SIZE) << " => "
            << as_string() << std::endl;
        return out;
    }
    if(type() == pretok_pretok){
        out << "    [l:" << to_string(line(), 4) << " c:"<< to_string(col(), 4) << "] "
            << align_right("pretok_pretok", PRETOK_SIZE) << " => "
            << "\"" << as_string() << "\" len:" << as_string().size()
            << std::endl;
        return out;
    }

    Block& bls = as_block();

    out << "[l:" << to_string(line(), 4) << " c:"<< to_string(col(), 4) << "] "
        << align_right("pretok_preblock", PRETOK_SIZE) << ": \n";

    for(auto& i:bls){
        //out << "    ";
        i.debug_print(out);
    }

    return out << std::flush;
}
std::ostream& PreToken::print(std::ostream& out){

    if(type() == pretok_prestring){
        out << "\"" << as_string() << "\"\n";
        return out;
    }

    if (type() == pretok_pretok){
        out << as_string() << "\n";
        return out;
    }

    Block& bls = as_block();

    out << "{\n";

    for(auto& i:bls){
        out << "    "; i.print(out);
    }

    out << "}\n";

    return out;
}

std::ostream& Prelexer::debug_print(std::ostream& out){

    while(_reader){
        next_pretoken().debug_print(out);
    }

    return out;
}
std::ostream& Prelexer::print(std::ostream& out){
    while(_reader){
        next_pretoken().print(out);
    }
    return out;
}

}
