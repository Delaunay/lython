#include "parser/parser.h"
#include "utilities/guard.h"

namespace lython {

NEW_EXCEPTION(PrimaryExpression);

Expression Parser::parse_type(Module& m, size_t depth) {
    TRACE_START();
    String name = "<typename>";
    WITH_EXPECT(tok_identifier, "expect type identifier") {
        name = token().identifier();
    }

    auto type = m.reference(name);
    type.start() = token();
    EAT(tok_identifier);

    TRACE_END();
    return type;
}

Token Parser::ignore_newlines() {
    Token tok = token();
    while (tok.type() == tok_newline) {
        tok = next_token();
    }
    return tok;
}

Expression Parser::parse_compound_statement(Module& m, std::size_t depth) {
    TRACE_START();
    // if a docstring is present the indent token was already eaten
    // EXPECT(tok_indent, "Expected start of compound statement");
    // EAT(tok_indent);

    auto expr = Expression::make<AST::SeqBlock>();
    auto *block = expr.ref<AST::SeqBlock>();
    Token tok = token();

    while (tok.type() != tok_desindent && tok.type() != tok_eof) {
        auto expr = parse_top_expression(m, depth + 1);

        block->blocks.push_back(expr);

        tok = ignore_newlines();
    }

    if (token().type() == tok_eof) {
        TRACE_END();
        return expr;
    }

    EXPECT(tok_desindent, "Expected end of compound statement");
    EAT(tok_desindent);
    TRACE_END();
    return expr;
}


Expression Parser::parse_primary(Module& m, std::size_t depth){
    TRACE_START();
    auto _ = guard([&](){
        TRACE_END();
    });

    Token tok = token();

    switch(tok.type()){
    // values
    case tok_int:{
        auto r = Expression::make<AST::Value>(tok.as_integer(), m.reference("Int"));
        next_token();
        return r;
    }

    case tok_string:{
        auto r = Expression::make<AST::Value>(tok.identifier(), m.reference("String"));
        next_token();
        return r;
    }

    case tok_float:{
        auto r = Expression::make<AST::Value>(tok.as_float(), m.reference("Float"));
        next_token();
        return r;
    }

    case '(': {
        next_token();
        auto r = parse_expression(m, depth + 1);
        EAT(')');
        return r;
    }
    // case indent ?
    // => compound statement

    // Unary Operator
    case tok_operator:{
        auto expr = parse_expression(m, depth + 1);
        return Expression::make<AST::UnaryOperator>(tok.operator_name(), expr);
    }

    // Function Call or Variable
    case tok_identifier:{
        auto ref = m.reference(tok.identifier());
        ref.start() = token();
        next_token();

        // Function
        if (token().type() == '('){
            return parse_function_call(m, ref, depth + 1);
        }

        // reference to a variable
        return ref;
    }

    // ---
    default:
        throw PrimaryExpression(
            "Expected {} got {}", to_string(tok.type()));
    }
}


Expression Parser::parse_expression(Module& m, std::size_t depth){
    auto lhs = parse_primary(m, depth);

    if (token().type() == tok_operator){
        return parse_expression_1(m, lhs, 0, depth);
    }

    return lhs;
}

Expression Parser::parse_expression_1(Module& m, Expression lhs, int precedence, std::size_t depth){
    TRACE_START();
    Token tok = token();
    EXPECT(tok_operator, "Expect an operator");

    static auto no_op = OpConfig();

    auto get_op_config = [m](String const& op) -> OpConfig const& {
        auto val = default_precedence().find(op);
        if (val != default_precedence().end()){
            return (*val).second;
        }
        return no_op;
    };

    // if not a binary op precedence is -1
    while (true){
        tok = token();

        String op;
        if (tok.type() == tok_operator){
            op = tok.operator_name();
        }

        OpConfig const& op_conf = get_op_config(op);
        if (op_conf.precedence < precedence){
            return lhs;
        }

        tok = next_token();
        auto rhs = parse_primary(m, depth + 1);

        if (!rhs){
            return Expression();
        }

        tok = token();
        String op2;
        if (tok.type() == tok_operator){
            op2 = tok.operator_name();
        }

        OpConfig const& op_conf2 = get_op_config(op2);
        // info("({}: {}) < ({} {})", op, op_conf.precedence, op2, op_conf2.precedence);
        if (op_conf.precedence < op_conf2.precedence){
            rhs = parse_expression_1(m, rhs, op_conf.precedence + 1, depth + 1);
            if (!rhs){
                return Expression();
            }
        }

        // bind operator operator
        if (op == "="){
            switch (lhs.kind()){
            // Default assign to a variable         A = ...
            case AST::NodeKind::KReference:{
                auto ref = lhs.ref<AST::Reference>();
                ref->index = 0;
                ref->length = 0;
                m.insert(ref->name.str(), rhs);
            }
            // TODO: Unpacking                      A, B, C = ....

            // Assign to an attribute               A.B = ....
            // we do not need to do anything the attribute is already in scope
            case AST::NodeKind::KBinaryOperator:
                break;

            default:
                debug("Unsupported assignment {} = {}", lhs, rhs);
                assert(false, "Unsupported assignment");
            }
        }

        lhs = Expression::make<AST::BinaryOperator>(
            lhs, rhs, get_string(op), op_conf.precedence);
    }
}

// return function call
Expression Parser::parse_function_call(Module& m, Expression function, std::size_t depth) {
    TRACE_START();
    auto expr = Expression::make<AST::Call>();
    auto fun = expr.ref<AST::Call>();

    fun->function = function;
    EXPECT('(', "`(` was expected");
    EAT('(');

    {
        int arg_count = 0;
        Module subscope = m.enter();
        while (token().type() != ')') {
            // insert dummy expression to get the correct idx
            fun->arguments.push_back(parse_expression(subscope, depth + 1));
            subscope.insert("", Expression());

            arg_count += 1;
            if (token().type() == ',') {
                next_token();
            }
        }
    }

    EXPECT(')', "`)` was expected");
    EAT(')');
    TRACE_END();
    return expr;
}

} // namespace lython
