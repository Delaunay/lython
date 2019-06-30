#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "../Logging/logging.h"
#include "../Utilities/optional.h"

#include "Module.h"
#include <iostream>

/*
 *  TODO:
 *      Handle tok_incorrect so the parser does not stop
 *      I need a more generic way to parse tokens since
 *      I will need to parse vector<Token> too
 *
 *      Some statistic about compiler memory usage would be nice
 *
 *  if definition 1 is incorrect
 *  The parser should be able to parse definition 2 (which is correct)
 *  correctly
 *
 * if a body is incorrect it has not impact if the function is not used
 *
 *  I can't have an incorrect identifier. Only Numbers can be incorrect
 *
 */

#define EAT(tok)                                                               \
    if (token().type() == tok) {                                               \
        next_token();                                                          \
    }

// assert(token().type() == tok && msg)
#define TRACE(out) out << "function " << std::endl;
#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg;

#define show_token(tok) debug(tok_to_string(tok.type()));
#define EXPECT(tok, msg) ASSERT(token().type() == tok, msg);

#define WITH_EXPECT(tok, msg)\
    if(token().type() != tok) {\
        error("Got (tok: %s, %d)", tok_to_string(token().type()).c_str(), token().type());\
        throw Exception(msg);\
    } else

namespace lython {

class Parser {
  public:
    Parser(AbstractBuffer &buffer) : _lex(buffer) {}

    // Shortcut
    Token next_token() { return _lex.next_token(); }
    Token token() { return _lex.token(); }

    ST::Expr get_unparsed_block() {}

    // currently type is a string
    // nevertheless we want to support advanced typing
    // which means type will be an expression too
    Type parse_type() {}

    std::string get_identifier() {
        if (token().type() == tok_identifier){
            return token().identifier();
        }

        debug("Missing identifier");
        return std::string("--expected-identifier--");
    }

    // Parsing routines
    ST::Expr parse_function(bool eager, int depth) {
        trace(depth, "Function");
        EAT(tok_def);

        // Get Name
        AST::Function *fun = new AST::Function(get_identifier());
        std::string ret_type = "unknown";

        Token tok = next_token();

        EXPECT('(', "( was expected"); EAT('(');

        debug("Parse Arguments");
        while (token().type() != ')' && token()) {

            std::string vname = CHECK_NAME(get_identifier());
            std::string type = "unknown";
            next_token();

            // type declaration
            if (token().type() == ':') {
                next_token();
                type = CHECK_TYPE(get_identifier());
                next_token();
            }

            if (token().type() == ',') {
                next_token();
            }

            // Add parameter
            fun->args().push_back(AST::Placeholder(vname, type));
        }
        EAT(')');
        // Read return type if any
        tok = token();
        if (tok.type() == tok_arrow) {
            EAT(tok_arrow);
            ret_type = CHECK_TYPE(get_identifier());

            tok = next_token();
        }
        fun->return_type() = make_type(ret_type);

        EXPECT(':', ": was expected")               ; EAT(':');
        EXPECT(tok_newline, "new line was expected"); EAT(tok_newline);
        EXPECT(tok_indent , "indent was expected")  ; EAT(tok_indent);
        tok = token();

        if (tok.type() == tok_docstring){
            fun->docstring() = tok.identifier();

            tok = next_token();
            EXPECT(tok_newline, "new line was expected"); EAT(tok_newline);
        }

        if (! eager){
            AST::UnparsedBlock *body = new AST::UnparsedBlock();

            while (token().type() != tok_desindent && token()) {
                body->tokens().push_back(token());
                next_token();
            }

            fun->body() = ST::Expr(body);
        } else {
            debug("Parse body");
            ST::Expr body = parse_block(depth + 1);
            fun->body() = body;
        }

        EXPECT(tok_desindent , "desindent was expected")  ; EAT(tok_desindent);
        return ST::Expr(fun);
    }

    ST::Expr parse_value(int depth){
        trace(depth, "Parse value (%s: %i)", tok_to_string(token().type()).c_str(), token().type());

        AST::Value* val = nullptr;
        int8 type = token().type();

        switch(type){
        case tok_string:
            val = new AST::Value(token().identifier(), make_type("string"));
            EAT(tok_string);
            break;
        case tok_float:
            val = new AST::Value(token().as_float(), make_type("float"));
            EAT(tok_float);
            break;
        case tok_int:
            val = new AST::Value(token().as_integer(), make_type("int"));
            EAT(tok_int);
            break;
        }
        return ST::Expr(val);
    }

    ST::Expr parse_statement(int8 statement, int depth){
        trace(depth, "Parse statement");

        EXPECT(statement, ": was expected"); EAT(statement);

        AST::Statement* stmt = new AST::Statement();
        stmt->statement() = statement;

        stmt->expr() = parse_expression(depth + 1);
        return ST::Expr(stmt);
    }

    ST::Expr parse_function_call(ST::Expr function, int depth){
        trace(depth, "Parse function call");
        AST::Call* fun = new AST::Call();
        fun->function() = function;

        EXPECT('(', "`(` was expected"); EAT('(');

        while(token().type() != ')'){
            fun->arguments().push_back(parse_expression(depth + 1));
        }

        EXPECT(')', "`)` was expected"); EAT(')');
        return ST::Expr(fun);
    }

    ST::Expr parse_operator(int depth){
        trace(depth, "Parse operator (%s: %i)", tok_to_string(token().type()).c_str(), token().type());

        ST::Expr lhs = parse_value(depth + 1);

        AST::Ref* op = new AST::Ref();
        if (token().type() == tok_identifier){
            op->name() = get_identifier();
            EAT(tok_identifier);
        } else{
            op->name() = token().type();
            next_token();
        }

        ST::Expr rhs = parse_value(depth + 1);
        AST::BinaryOperator* bin = new AST::BinaryOperator(rhs, lhs, ST::Expr(op));

        return ST::Expr(bin);
    }

    // parse function_name(args...)
    ST::Expr parse_expression(int depth){
        trace(depth, "Parse Expression (%s: %i)", tok_to_string(token().type()).c_str(), token().type());

        switch(token().type()){
            case tok_async:
            case tok_yield:
            case tok_return:
                return parse_statement(token().type(), depth + 1);
                break;

            case tok_def:
                return parse_function(true, depth + 1);

            // value probably an operation X + Y
            case tok_identifier:{
                AST::Ref* fun_name = new AST::Ref();
                fun_name->name() = get_identifier(); EAT(tok_identifier);
                return parse_function_call(ST::Expr(fun_name), depth + 1);
            }/*
            case tok_string:
            case tok_int:
            case tok_float:
                return parse_value();*/

            default:
                return parse_operator(depth + 1);
            /*
                AST::Ref* name = new AST::Ref();
                name->name() += token().type();
                next_token();
                return ST::Expr(name);*/
        }
    }

    ST::Expr parse_struct(int depth){
        trace(depth, "Parse Struct");
        EAT(tok_struct);

        Token tok = token();
        EXPECT(tok_identifier, "Expect an identifier");
        std::string name = tok.identifier(); EAT(tok_identifier);

        AST::Struct* data = new AST::Struct();
        data->name() = name;

        EXPECT(':', ": was expected"); EAT(':');
        EXPECT(tok_newline, "newline was expected"); EAT(tok_newline);
        EXPECT(tok_indent, "indentation was expected"); EAT(tok_indent);

        tok = token();

        // docstring
        if (tok.type() == tok_docstring){
            data->docstring() = tok.identifier();
            tok = next_token();
        }

        while(tok.type() == tok_newline){
            tok = next_token();
        }

        tok = token();
        while(tok.type() != tok_desindent && tok.type() != tok_eof){

            WITH_EXPECT(tok_identifier, "Expected identifier 1"){
                name = tok.identifier(); EAT(tok_identifier);
            }

            EXPECT(':', "Expect :"); EAT(':');
            tok = token();

            WITH_EXPECT(tok_identifier, "Expect type identifier"){
                AST::Ref* ref = new AST::Ref();
                ref->name() = tok.identifier();
                data->attributes()[name] = ST::Expr(ref);
                EAT(tok_identifier);
                tok = token();
            }

            while(tok.type() == tok_newline){
                tok = next_token();
            }

        }

        return ST::Expr(data);
    }

    ST::Expr parse_block(int depth){
        trace(depth, "Parse block");

        AST::SeqBlock* block = new AST::SeqBlock();

        Token tok = token();

        while (tok.type() != tok_desindent){
            auto expr = parse_expression(depth + 1);
            block->blocks().push_back(expr);

            tok = token();
            while (tok.type() == tok_newline){
                tok = next_token();
            }
        }

        return ST::Expr(block);
    }

    // return One Top level Expression (Functions)
    ST::Expr parse_one(int depth = 0) {
        Token tok = token();
        if (tok.type() == tok_incorrect){
            tok = next_token();
        }

        while (tok.type() == tok_newline){
            tok = next_token();
        }

        switch (tok.type()) {

        case tok_def:
            return parse_function(true, depth);

        case tok_struct:
            return parse_struct(depth);

        default:
            assert("Unknown Token");
        }
    }

    //
    BaseScope parse_all() {
        // first token is tok_incorrect
        while (token()) {
            _scope.insert(parse_one());
        }
    }

  private:
    Lexer _lex;
    BaseScope _scope;
};

} // namespace lython

#endif // PARSER_H
