#include "Prelexer.h"
#include "../fmt.h"

#define PRETOK_SIZE 10

namespace lython {

std::ostream& PreToken::debug_print(std::ostream& out){
    //static int indent_level = 0;

    if(type() == pretok_prestring){
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("prestring", PRETOK_SIZE) << " => "
            << as_string() << std::endl;
        return out;
    }
    if(type() == pretok_pretok){
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("pretok", PRETOK_SIZE) << " => "
            << "\"" << as_string() << "\" len:" << as_string().size()
            << std::endl;
        return out;
    }

    Block& bls = as_block();

    out << MGREEN "[l:" << to_string(line_begin(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
        << align_right("preblock", PRETOK_SIZE)
        << MGREEN " [l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
        << ": \n";

    for(auto& i:bls){
        if (i.type() != pretok_preblock)
            out << "    ";
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
