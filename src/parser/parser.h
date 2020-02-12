#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../logging/logging.h"

#include "../utilities/optional.h"
#include "../utilities/stack.h"
#include "../utilities/trie.h"
#include "../utilities/metadata.h"

#include "../ast/nodes.h"
#include "module.h"

#include <iostream>
#include <numeric>

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
 * if a body is incorrect it has no impact if the function is not used
 *
 *  I can't have an incorrect identifier. Only Numbers can be incorrect
 *
 */

// "" at the end of the macro remove a warning about unecessary ;
#define EAT(tok)                                                               \
    if (token().type() == tok) {                                               \
        next_token();                                                          \
    }                                                                          \
    ""

// assert(token().type() == tok && msg)
#define TRACE_START()                                                          \
    trace_start(depth, "({}: {})", tok_to_string(token().type()).c_str(),      \
                token().type())
#define TRACE_END()                                                            \
    trace_end(depth, "({}: {})", tok_to_string(token().type()).c_str(),        \
              token().type())

#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg

#define show_token(tok) debug(tok_to_string(tok.type()))
#define EXPECT(tok, msg)                                                       \
    ASSERT(token().type() == tok, msg);                                        \
    ""


namespace lython {

class ParserException : public std::exception {
  public:
    ParserException(String const &msg) : msg(msg) {}

    const char *what() const noexcept override { return msg.c_str(); }

    const String msg;
};

#define WITH_EXPECT(tok, msg)                                                  \
    if (token().type() != tok) {                                               \
        debug("Got (tok: {}, {})", tok_to_string(token().type()).c_str(),      \
              token().type());                                                 \
        throw ParserException(msg);                                            \
    } else


class Parser {
  public:
    Parser(AbstractBuffer &buffer, Module *module)
        : module(module), _lex(buffer) {
        metadata_init_names();
    }

    // Shortcut
    Token next_token()  { return _lex.next_token(); }
    Token token()       { return _lex.token();      }
    Token peek_token()  { return _lex.peek_token(); }

    String get_identifier() {
        if (token().type() == tok_identifier) {
            return token().identifier();
        }

        debug("Missing identifier");
        return String("<identifier>");
    }

    /*  <function-definition> ::= {<declaration-specifier>}* <declarator>
     * {<declaration>}* <compound-statement>
     */
    Expression parse_function(Module& m, std::size_t depth);

    Expression parse_compound_statement(Module& m, std::size_t depth);

    Token ignore_newlines();

    Expression parse_type(Module& m, std::size_t depth);

    AST::ParameterList parse_parameter_list(Module& m, std::size_t depth);

    Expression parse_value(Module& m, std::size_t depth) {
        TRACE_START();

        Expression val;
        int8 type = token().type();

        switch (type) {
        case tok_string:
            // make_type("string")
            val =  Expression::make<AST::Value>(token().identifier(), Expression());
            EAT(tok_string);
            break;
        case tok_float:
            // make_type("float")
            val =  Expression::make<AST::Value>(token().as_float(), Expression());
            EAT(tok_float);
            break;
        case tok_int:
            // make_type("int")
            val = Expression::make<AST::Value>(token().as_integer(), Expression());
            EAT(tok_int);
            break;
        }

        // are we done ?
        auto ttype = token().type();
        if (ttype == tok_newline || ttype == tok_eof || ttype == ',' ||
            ttype == ')')
            return Expression(val);


        String name;
        if (token().type() == tok_identifier) {
            name = get_identifier();
            EAT(tok_identifier);
        } else {
            name = token().type();
            next_token();
        }

        int loc = m.find_index(name);
        int size = m.size();

        if (loc < 0){
            warn("Undefined type \"{}\"", name.c_str());
        }
        // We are not creating a reference here
        // we are using a reference that was created before
        //auto op = new AST::Ref(name, loc, size);
        auto op = m.find(name);

        Expression rhs = parse_value(m, depth + 1);
        Expression bin = Expression::make<AST::BinaryOperator>(rhs, val, op);
        TRACE_END();
        return Expression(bin);
    }

    Expression parse_statement(Module& m, int8 statement, std::size_t depth) {
        TRACE_START();
        EXPECT(statement, ": was expected");
        EAT(statement);

        auto expr = Expression::make<AST::Statement>();
        auto stmt = expr.ref<AST::Statement>();
        stmt->statement = statement;

        stmt->expr = parse_expression(m, depth + 1);
        return expr;
    }

    Expression parse_function_call(Module& m, Expression function, std::size_t depth) {
        TRACE_START();
        auto expr = Expression::make<AST::Call>();
        auto fun = expr.ref<AST::Call>();

        fun->function = function;

        EXPECT('(', "`(` was expected");
        EAT('(');

        while (token().type() != ')') {
            // token().debug_print(std::cout);

            fun->arguments.push_back(parse_expression(m, depth + 1));

            if (token().type() == ',') {
                next_token();
            }
        }

        EXPECT(')', "`)` was expected");
        EAT(')');
        return expr;
    }

    String parse_operator() {
        Trie<128> const *iter = nullptr;
        String op_name;

        // Operator is a string
        // Currently this code path is not used
        // not sure if we should allow identifier to be operators
        if (token().type() == tok_identifier) {
            iter = module->operator_trie();
            iter = iter->matching(token().identifier());

            if (iter != nullptr) {
                if (iter->leaf()) {
                    op_name = token().identifier();
                    debug("Operator is string");
                } else {
                    warn("Operator {} was not found, did you mean: ...",
                         token().identifier().c_str());
                }
            }
        } // ----
        else {
            // debug("Operator parsing");
            iter = module->operator_trie();

            bool operator_parsing = true;
            while (operator_parsing) {
                // debug("Tok is %c", char(token().type()));

                // check next token
                iter = iter->matching(token().type());

                if (iter != nullptr) {
                    // debug("Added tok to operator %c", char(token().type()));
                    op_name.push_back(token().type());
                    next_token();
                } else {
                    warn("Could not match {} {}", char(token().type()),
                         token().type());
                }

                // token that stop operator parsing
                switch (token().type()) {
                case '(':
                case ')':
                case tok_identifier:
                case tok_float:
                case tok_int:
                case tok_newline:
                    operator_parsing = false;
                    // debug("Found terminal tok %c", char(token().type()));
                    break;
                }
            }

            if (!iter->leaf()) {
                warn("Operator {} was not found, did you mean: ...",
                     op_name.c_str());
            }
        }
        return op_name;
    }

    // Shunting-yard_algorithm
    // Parse a full line of function and stuff
    Expression parse_expression(Module& m, std::size_t depth);

    // parse function_name(args...)
    Expression parse_top_expression(Module& m, std::size_t depth) {
        TRACE_START();

        switch (token().type()) {
        case tok_async:
        case tok_yield:
        case tok_return:
            return parse_statement(m, token().type(), depth + 1);

        case tok_def:
            return parse_function(m, depth + 1);

        // value probably an operation X + Y
        //            case tok_identifier:{
        //                AST::Ref* fun_name = new AST::Ref();
        //                fun_name->name() = token().identifier();
        //                EAT(tok_identifier);
        //                return parse_function_call(Expression(fun_name), depth +
        //                1);
        //            }

        case tok_struct:
            return parse_struct(m, depth + 1);

        case tok_identifier:
        case tok_string:
        case tok_int:
        case tok_float:
            return parse_expression(m, depth + 1);

            //            default:
            //                return parse_operator(depth + 1);
        }

        return Expression();
    }

    /*  <struct-or-union> ::= struct | union
        <struct-or-union-specifier> ::= <struct-or-union> <identifier> {
       {<struct-declaration>}+ }
                                      | <struct-or-union> {
       {<struct-declaration>}+ }
                                      | <struct-or-union> <identifier>
     */
    Expression parse_struct(Module& m, std::size_t depth) {
        TRACE_START();
        EAT(tok_struct);

        Token tok = token();
        EXPECT(tok_identifier, "Expect an identifier");
        String struct_name = tok.identifier();
        EAT(tok_identifier);

        auto struct_ = Expression::make<AST::Struct>(struct_name);
        auto *data = struct_.ref<AST::Struct>();

        EXPECT(':', ": was expected");
        EAT(':');
        EXPECT(tok_newline, "newline was expected");
        EAT(tok_newline);

        EXPECT(tok_indent, "indentation was expected");
        EAT(tok_indent);

        tok = token();

        // docstring
        if (tok.type() == tok_docstring) {
            data->docstring = tok.identifier();
            tok = next_token();
        }

        while (tok.type() == tok_newline) {
            tok = next_token();
        }

        tok = token();
        while (tok.type() != tok_desindent && tok.type() != tok_eof) {
            String attribute_name = "<attribute>";

            WITH_EXPECT(tok_identifier, "Expected identifier 1") {
                attribute_name = tok.identifier();
                EAT(tok_identifier);
            }

            EXPECT(':', "Expect :");
            EAT(':');

            data->attributes[attribute_name] = parse_type(m, depth);
            tok = token();

            while (tok.type() == tok_newline) {
                tok = next_token();
            }
        }

        module->insert(struct_name, struct_);
        return struct_;
    }

    // return One Top level Expression (Functions)
    Expression parse_one(Module& m, std::size_t depth = 0) {
        Token tok = token();
        if (tok.type() == tok_incorrect) {
            tok = next_token();
        }

        while (tok.type() == tok_newline) {
            tok = next_token();
        }

        switch (tok.type()) {
        case tok_def:
            return parse_function(m, depth);

        case tok_struct:
            return parse_struct(m, depth);

        default:
            assert("Unknown Token");
        }

        return Expression();
    }

    //
//    BaseScope parse_all() {
//        // first token is tok_incorrect
//        while (token()) {
//            _scope.insert(parse_one());
//        }
//    }

  private:
    // Top Level Module
    Module *module;
    Lexer _lex;
    // BaseScope _scope;
};

} // namespace lython

#endif // PARSER_H
