#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "../Logging/logging.h"
#include "../Utilities/optional.h"

#include "Module.h"

#include <iostream>
#include <unordered_set>
#include <unordered_map>

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
#define TRACE_START() trace_start(depth, "(%s: %i)", tok_to_string(token().type()).c_str(), token().type());
#define TRACE_END() trace_end(depth, "(%s: %i)", tok_to_string(token().type()).c_str(), token().type());

#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg;

#define show_token(tok) debug(tok_to_string(tok.type()));
#define EXPECT(tok, msg) ASSERT(token().type() == tok, msg);

#define WITH_EXPECT(tok, msg)\
    if(token().type() != tok) {\
        debug("Got (tok: %s, %d)", tok_to_string(token().type()).c_str(), token().type());\
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

    ST::Expr parse_unary_operator(){
        static const std::unordered_set<char> operators{'&', '*', '+', '-', '~', '!'};

        Token tok = token();
        int8 count = int8(operators.count(char(tok.type())));
        if (count == 1){
            auto unary_op = new AST::UnaryOperator();
            unary_op->operation() = tok.type();
            return ST::Expr(unary_op);
        }
    }

    std::string get_identifier() {
        if (token().type() == tok_identifier){
            return token().identifier();
        }

        debug("Missing identifier");
        return std::string("<identifier>");
    }


    ST::Expr parse_storage_class_specifier(){
        static const std::unordered_set<std::string> storage{
            "auto", "register", "static", "extern", "typedef"
        };

        Token tok = token();
        int8 count = int8(storage.count(tok.identifier()));
        if (count == 1){
            //
        }
    }

    ST::Expr parse_type_specifier(){
        static const std::unordered_set<std::string> type_specifier{
            "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned"
        }; // + struct | union | enum | typedef
    }


    /* <declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier><declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>
     */
    ST::Expr parse_declaration_specifier(){

    }

    /*  <function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>
     */
    ST::Expr parse_function(std::size_t depth);

    ST::Expr parse_compound_statement(std::size_t depth);

    Token ignore_newlines();

    ST::Expr parse_type(std::size_t depth);

    AST::ParameterList parse_parameter_list(std::size_t depth);

    ST::Expr parse_value(std::size_t depth){
        TRACE_START();

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

        // are we done ?
        if (token().type() == tok_newline || token().type() == tok_eof)
            return ST::Expr(val);

        auto op = new AST::Ref();
        if (token().type() == tok_identifier){
            op->name() = get_identifier();
            EAT(tok_identifier);
        } else{
            op->name() = token().type();
            next_token();
        }

        ST::Expr rhs = parse_value(depth + 1);
        AST::BinaryOperator* bin = new AST::BinaryOperator(rhs, ST::Expr(val), ST::Expr(op));
        TRACE_END();
        return ST::Expr(bin);
    }

    ST::Expr parse_statement(int8 statement, std::size_t depth){
        TRACE_START();
        EXPECT(statement, ": was expected"); EAT(statement);

        auto* stmt = new AST::Statement();
        stmt->statement() = statement;

        stmt->expr() = parse_expression(depth + 1);
        return ST::Expr(stmt);
    }

    ST::Expr parse_function_call(ST::Expr function, std::size_t depth){
        TRACE_START();
        auto fun = new AST::Call();
        fun->function() = function;

        EXPECT('(', "`(` was expected"); EAT('(');

        while(token().type() != ')'){
            fun->arguments().push_back(parse_expression(depth + 1));
        }

        EXPECT(')', "`)` was expected"); EAT(')');
        return ST::Expr(fun);
    }

    ST::Expr parse_operator(std::size_t depth){
        TRACE_START();
        ST::Expr lhs = parse_value(depth + 1);

        auto op = new AST::Ref();
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
    ST::Expr parse_expression(std::size_t depth){
        TRACE_START();

        switch(token().type()){
            case tok_async:
            case tok_yield:
            case tok_return:
                return parse_statement(token().type(), depth + 1);
                break;

            case tok_def:
                return parse_function(depth + 1);

            // value probably an operation X + Y
            case tok_identifier:{
                AST::Ref* fun_name = new AST::Ref();
                fun_name->name() = get_identifier(); EAT(tok_identifier);
                return parse_function_call(ST::Expr(fun_name), depth + 1);
            }
            case tok_string:
            case tok_int:
            case tok_float:
                return parse_value(depth + 1);

            default:
                return parse_operator(depth + 1);
        }
    }

    /*  <struct-or-union> ::= struct | union
        <struct-or-union-specifier> ::= <struct-or-union> <identifier> { {<struct-declaration>}+ }
                                      | <struct-or-union> { {<struct-declaration>}+ }
                                      | <struct-or-union> <identifier>
     */
    ST::Expr parse_struct(std::size_t depth){
        TRACE_START();
        EAT(tok_struct);

        Token tok = token();
        EXPECT(tok_identifier, "Expect an identifier");
        std::string name = tok.identifier(); EAT(tok_identifier);

        auto* data = new AST::Struct();
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

    ST::Expr parse_block(std::size_t depth){
        TRACE_START();

        auto* block = new AST::SeqBlock();
        Token tok = token();

        while (tok.type() != tok_desindent && tok.type() != tok_eof){
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
    ST::Expr parse_one(std::size_t depth = 0) {
        Token tok = token();
        if (tok.type() == tok_incorrect){
            tok = next_token();
        }

        while (tok.type() == tok_newline){
            tok = next_token();
        }

        switch (tok.type()) {
        case tok_def:
            return parse_function(depth);

        case tok_struct:
            return parse_struct(depth);

        default:
            assert("Unknown Token");
        }
    }

    /*
     * <external-declaration> ::= <function-definition>
     *                          | <declaration>
     */
    ST::Expr parse_external_declaration(){

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
