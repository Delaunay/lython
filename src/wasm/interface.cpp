#ifndef BUILD_WEBASSEMBLY
#define BUILD_WEBASSEMBLY 1
#endif

#if BUILD_WEBASSEMBLY
#include <emscripten/bind.h>
#include <emscripten/val.h>


#include "sema/sema.h"
#include "parser/parser.h"
#include "vm/tree.h"


using namespace emscripten;
using namespace lython;

// class_<SemanticAnalyser>("SemanticAnalyser")
//     .constructor<int, std::string>()
//     .function("incrementX", &MyClass::incrementX)
//     .property("x", &MyClass::getX, &MyClass::setX)
//     .class_function("getStringFromInstance", &MyClass::getStringFromInstance)
// ;


String make_string(std::string const& str) {
    return String(std::begin(str), std::end(str));
}

// Buffer
EMSCRIPTEN_BINDINGS(lython) {

    class_<Node>("Node");
    class_<Module>("Module");


    class_<String>("String")
        .constructor(make_string)
    ;

    class_<StringBuffer>("StringBuffer")
        .constructor<String>()
        .function("next", &StringBuffer::getc)
        .function("current", &StringBuffer::peek)
    ;

    // = ;

    class_<Token>("Token")
        .function("line", &Token::line)
        .function("col", &Token::col)
        .function("type", &Token::type)
        .function("identifier", select_overload<String const&() const>(&Token::identifier))
    ;

    class_<Lexer>("Lexer")
        .constructor<StringBuffer&>()
        .function("next", &Lexer::next_token)
        .function("peek", &Lexer::peek_token)
        .function("current", &Lexer::token)
    ;

    // return_value_policy::reference
    class_<Parser>("Parser")
        .constructor<Lexer&>()
        .function("parse_module", &Parser::parse_module, return_value_policy::take_ownership())
        .function("next", &Parser::next, return_value_policy::reference())
    ;

    class_<SemaException>("SemaException")
    ;

    SemaVisitorTrait::ModRet(SemanticAnalyser::*method)(Module*, int) = &SemanticAnalyser::module;

    class_<SemanticAnalyser>("SemanticAnalyser")
        .constructor<>()
        .function("analyse", method, allow_raw_pointers())
        .function("has_errors", &SemanticAnalyser::has_errors)
    ;
}


#endif