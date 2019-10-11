#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "../Logging/logging.h"
#include "../Types.h"
#include "../Utilities/optional.h"
#include "../Utilities/stack.h"
#include "../Utilities/trie.h"

#include "Module.h"

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
    trace_start(depth, "(%s: %i)", tok_to_string(token().type()).c_str(),      \
                token().type())
#define TRACE_END()                                                            \
    trace_end(depth, "(%s: %i)", tok_to_string(token().type()).c_str(),        \
              token().type())

#define CHECK_TYPE(type) type
#define CHECK_NAME(name) name
#define PARSE_ERROR(msg) std::cout << msg

#define show_token(tok) debug(tok_to_string(tok.type()))
#define EXPECT(tok, msg)                                                       \
    ASSERT(token().type() == tok, msg);                                        \
    ""

class ParserException : public std::exception {
  public:
    ParserException(std::string const &msg) : msg(msg) {}

    const char *what() const noexcept override { return msg.c_str(); }

    const std::string msg;
};

#define WITH_EXPECT(tok, msg)                                                  \
    if (token().type() != tok) {                                               \
        debug("Got (tok: %s, %d)", tok_to_string(token().type()).c_str(),      \
              token().type());                                                 \
        throw ParserException(msg);                                            \
    } else

namespace lython {

class Parser {
  public:
    Parser(AbstractBuffer &buffer, Module *module)
        : module(module), _lex(buffer) {}

    // Shortcut
    Token next_token() { return _lex.next_token(); }
    Token token() { return _lex.token(); }
    Token peek_token() { return _lex.peek_token(); }

    ST::Expr get_unparsed_block() {}

    // currently type is a string
    // nevertheless we want to support advanced typing
    // which means type will be an expression too
    ST::Expr parse_type() {}

    ST::Expr parse_unary_operator() {
        static const std::unordered_set<char> operators{'&', '*', '+',
                                                        '-', '~', '!'};

        Token tok = token();
        int8 count = int8(operators.count(char(tok.type())));
        if (count == 1) {
            auto unary_op = new AST::UnaryOperator();
            unary_op->operation() = tok.type();
            return ST::Expr(unary_op);
        }
    }

    std::string get_identifier() {
        if (token().type() == tok_identifier) {
            return token().identifier();
        }

        debug("Missing identifier");
        return std::string("<identifier>");
    }

    ST::Expr parse_storage_class_specifier() {
        static const std::unordered_set<std::string> storage{
            "auto", "register", "static", "extern", "typedef"};

        Token tok = token();
        int8 count = int8(storage.count(tok.identifier()));
        if (count == 1) {
            //
        }
    }

    ST::Expr parse_type_specifier() {
        static const std::unordered_set<std::string> type_specifier{
            "void",  "char",   "short",  "int",     "long",
            "float", "double", "signed", "unsigned"}; // + struct | union | enum
                                                      // | typedef
    }

    /* <declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier><declaration-specifier> ::=
       <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>
     */
    ST::Expr parse_declaration_specifier() {}

    /*  <function-definition> ::= {<declaration-specifier>}* <declarator>
     * {<declaration>}* <compound-statement>
     */
    ST::Expr parse_function(std::size_t depth);

    ST::Expr parse_compound_statement(std::size_t depth);

    Token ignore_newlines();

    ST::Expr parse_type(std::size_t depth);

    AST::ParameterList parse_parameter_list(std::size_t depth);

    ST::Expr parse_value(std::size_t depth) {
        TRACE_START();

        AST::Value *val = nullptr;
        int8 type = token().type();

        switch (type) {
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
        auto ttype = token().type();
        if (ttype == tok_newline || ttype == tok_eof || ttype == ',' ||
            ttype == ')')
            return ST::Expr(val);

        auto op = new AST::Ref();
        if (token().type() == tok_identifier) {
            op->name() = get_identifier();
            EAT(tok_identifier);
        } else {
            op->name() = token().type();
            next_token();
        }

        ST::Expr rhs = parse_value(depth + 1);
        AST::BinaryOperator *bin =
            new AST::BinaryOperator(rhs, ST::Expr(val), ST::Expr(op));
        TRACE_END();
        return ST::Expr(bin);
    }

    ST::Expr parse_statement(int8 statement, std::size_t depth) {
        TRACE_START();
        EXPECT(statement, ": was expected");
        EAT(statement);

        auto *stmt = new AST::Statement();
        stmt->statement() = statement;

        stmt->expr() = parse_expression(depth + 1);
        return ST::Expr(stmt);
    }

    ST::Expr parse_function_call(ST::Expr function, std::size_t depth) {
        TRACE_START();
        auto fun = new AST::Call();
        fun->function() = function;

        EXPECT('(', "`(` was expected");
        EAT('(');

        while (token().type() != ')') {
            // token().debug_print(std::cout);

            fun->arguments().push_back(parse_expression(depth + 1));

            if (token().type() == ',') {
                next_token();
            }
        }

        EXPECT(')', "`)` was expected");
        EAT(')');
        return ST::Expr(fun);
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
                    warn("Operator %s was not found, did you mean: ...",
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
                    warn("Could not match %c %d", char(token().type()),
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
                warn("Operator %s was not found, did you mean: ...",
                     op_name.c_str());
            }
        }
        return op_name;
    }

    // Shunting-yard_algorithm
    // Parse a full line of function and stuff
    ST::Expr parse_expression(std::size_t depth) {
        TRACE_START();
        Stack<AST::MathNode> output_stack;
        Stack<AST::MathNode> operator_stack;
        AST::MathNode node;

        while (token().type() != tok_newline) {
            Token tok = token();
            if (tok.type() == ',') {
                tok = next_token();
            }

            // if a variable or number is found, it is copied directly to the
            // output
            switch (tok.type()) {

            // function call
            case tok_identifier: {
                Token tok2 = peek_token();
                debug("peek_token = %d %c %s", token().type(),
                      char(token().type()), token().identifier().c_str());

                if (tok2.type() == '(') {
                    // check if the function exists
                    auto function = module->find(tok.identifier());
                    int nargs = 1;

                    // fun
                    if (function != nullptr &&
                        function->kind() == AST::Expression::KindFunction) {
                        AST::Function *fun =
                            static_cast<AST::Function *>(function.get());
                        nargs = int(fun->args().size());
                    } else {
                        debug("function %s was not declared",
                              tok.identifier().c_str());
                    }
                    operator_stack.push(
                        {AST::MathKind::Function, tok.identifier(), nargs});

                    debug("push %s to operator", tok.identifier().c_str());
                    next_token();
                    continue;
                }
                // if it is not a function, it is a variable
                [[fallthrough]];
            }
            case tok_float:
            case tok_int:
                output_stack.push({AST::MathKind::Value, tok.identifier()});
                EAT(tok.type());
                debug("Added %s in output stack", tok.identifier().c_str());
                continue;
            }

            // If the symbol is an operator, it is pushed onto the operator
            // stack
            // If the operator's precedence is less than that of the operators
            // at the top of the stack
            // or the precedences are equal and the operator is left associative
            // then that operator is popped off the stack and added to the outpu

            // parse an operator
            // -------------------------------
            if (token().type() != '(' && token().type() != ')') {
                String op_name = parse_operator();

                if (operator_stack.size() > 0) {
                    int current_pred;
                    bool current_asso;
                    int operator_pred;
                    bool is_left_asso;

                    std::tie(current_pred, current_asso) =
                        module->precedence_table()[op_name];

                    node = operator_stack.peek();
                    std::tie(operator_pred, is_left_asso) =
                        module->precedence_table()[node.name];

                    while ((node.kind == AST::MathKind::Function ||
                            (operator_pred > current_pred) ||
                            (operator_pred == current_pred && is_left_asso)) &&
                           (node.name != "(")) {
                        // debug("%s is_fun %d | %d > %d | is_left_asso %d",
                        // cop.c_str(), is_fun, operator_pred, current_pred,
                        // is_left_asso);

                        operator_stack.pop();
                        debug("pop %s to operator", node.name.c_str());

                        output_stack.push(node);
                        debug("push %s to output", node.name.c_str());

                        if (operator_stack.size() == 0) {
                            break;
                        }

                        node = operator_stack.peek();
                        std::tie(operator_pred, is_left_asso) =
                            module->precedence_table()[node.name];
                    }
                }
                debug("push %s to operator", op_name.c_str());
                operator_stack.push({AST::MathKind::Operator, op_name});
            }

            if (tok.type() == '(') {
                operator_stack.push({AST::MathKind::None, "("});
                debug("push %s to operator", "(");
                next_token();
            } else if (tok.type() == ')') {
                node = operator_stack.peek();
                while (node.name != "(" && operator_stack.size() > 0) {

                    output_stack.push(operator_stack.pop());
                    node = operator_stack.peek();
                };

                if (node.name != "(") {
                    warn("mismatched parentheses");
                } else {
                    operator_stack.pop();
                }
                next_token();
            }
        }

        // Finally, any remaining operators are popped off the stack and added
        // to the output
        while (operator_stack.size() > 0) {
            output_stack.push(operator_stack.pop());
        }

        auto riter = output_stack.rbegin();
        while (riter != output_stack.rend()) {
            std::cout << (*riter).name << " ";
            ++riter;
        }
        std::cout << "\n";

        auto iter = output_stack.begin();
        while (iter != output_stack.end()) {
            auto op = *iter;

            std::cout << (*iter).name << " ";
            ++iter;
        }
        std::cout << "\n";

        auto expr = new AST::ReversePolishExpression(output_stack);
        return ST::Expr(expr);
    }

    ST::Expr parse_operator(std::size_t depth) {
        TRACE_START();
        ST::Expr lhs = parse_value(depth + 1);

        auto op = new AST::Ref();
        if (token().type() == tok_identifier) {
            op->name() = get_identifier();
            EAT(tok_identifier);
        } else {
            op->name() = token().type();
            next_token();
        }

        ST::Expr rhs = parse_value(depth + 1);
        AST::BinaryOperator *bin =
            new AST::BinaryOperator(rhs, lhs, ST::Expr(op));

        return ST::Expr(bin);
    }

    // parse function_name(args...)
    ST::Expr parse_top_expression(std::size_t depth) {
        TRACE_START();

        switch (token().type()) {
        case tok_async:
        case tok_yield:
        case tok_return:
            return parse_statement(token().type(), depth + 1);

        case tok_def:
            return parse_function(depth + 1);

        // value probably an operation X + Y
        //            case tok_identifier:{
        //                AST::Ref* fun_name = new AST::Ref();
        //                fun_name->name() = token().identifier();
        //                EAT(tok_identifier);
        //                return parse_function_call(ST::Expr(fun_name), depth +
        //                1);
        //            }

        case tok_struct:
            return parse_struct(depth + 1);

        case tok_identifier:
        case tok_string:
        case tok_int:
        case tok_float:
            return parse_expression(depth + 1);

            //            default:
            //                return parse_operator(depth + 1);
        }
    }

    /*  <struct-or-union> ::= struct | union
        <struct-or-union-specifier> ::= <struct-or-union> <identifier> {
       {<struct-declaration>}+ }
                                      | <struct-or-union> {
       {<struct-declaration>}+ }
                                      | <struct-or-union> <identifier>
     */
    ST::Expr parse_struct(std::size_t depth) {
        TRACE_START();
        EAT(tok_struct);

        Token tok = token();
        EXPECT(tok_identifier, "Expect an identifier");
        std::string name = tok.identifier();
        EAT(tok_identifier);

        auto *data = new AST::Struct();
        data->name() = name;

        EXPECT(':', ": was expected");
        EAT(':');
        EXPECT(tok_newline, "newline was expected");
        EAT(tok_newline);
        EXPECT(tok_indent, "indentation was expected");
        EAT(tok_indent);

        tok = token();

        // docstring
        if (tok.type() == tok_docstring) {
            data->docstring() = tok.identifier();
            tok = next_token();
        }

        while (tok.type() == tok_newline) {
            tok = next_token();
        }

        tok = token();
        while (tok.type() != tok_desindent && tok.type() != tok_eof) {

            WITH_EXPECT(tok_identifier, "Expected identifier 1") {
                name = tok.identifier();
                EAT(tok_identifier);
            }

            EXPECT(':', "Expect :");
            EAT(':');
            tok = token();

            WITH_EXPECT(tok_identifier, "Expect type identifier") {
                AST::Ref *ref = new AST::Ref();
                ref->name() = tok.identifier();
                data->attributes()[name] = ST::Expr(ref);
                EAT(tok_identifier);
                tok = token();
            }

            while (tok.type() == tok_newline) {
                tok = next_token();
            }
        }

        return module->insert(data);
    }

    // return One Top level Expression (Functions)
    ST::Expr parse_one(std::size_t depth = 0) {
        Token tok = token();
        if (tok.type() == tok_incorrect) {
            tok = next_token();
        }

        while (tok.type() == tok_newline) {
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
    ST::Expr parse_external_declaration() {}

    //
    BaseScope parse_all() {
        // first token is tok_incorrect
        while (token()) {
            _scope.insert(parse_one());
        }
    }

  private:
    Module *module;
    Lexer _lex;
    BaseScope _scope;
};

} // namespace lython

#endif // PARSER_H
