
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "parser/format_spec.h"
#include "utilities/strings.cpp"


namespace lython {

    // from the buffer create a list of nodes that could be created
    //  def -> function / name
    //
    // Have a context so we know 
    // When we are inside an expression

    enum class InputContext {
        Statement,
        Expression,
        Pattern,
        String,
    };

    struct InputNode {
        InputContext Context;
    };


    struct FunctionInput: public InputNode {
        InputNode name;
        // args
        // Body
    };

    class InteractiveBuffer: public AbstractBuffer {



        char getc() override {
            if (buffer.size() > 0)
             return char(std::getc(stdin)); 
        }

        String buffer;
    };


    void interactive_parser() {
        StringBuffer reader(code);
        Lexer  lex(reader);
        Parser parser(lex);
    }

}