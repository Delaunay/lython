#include "parser/parser.h"

namespace lython {

NEW_EXCEPTION(PrimaryExpression);

// Math Nodes for ReverPolish parsing
enum class MathKind {
    Operator,
    Value,
    Function,
    VarRef,
    None
};

struct MathNode {
    MathKind kind;
    int arg_count = 1;
    Expression ref;
    String name = "";
};

Expression Parser::parse_primary(Module& m, std::size_t depth){
    TRACE_START();

    Token tok = token();
    debug("tok {}", to_string(tok.type()));

    switch(tok.type()){
    // values
    case tok_int:{
        auto r = Expression::make<AST::Value>(tok.as_integer(), m.find("Int"));
        next_token();
        return r;
    }

    case tok_string:{
        auto r = Expression::make<AST::Value>(tok.identifier(), m.find("String"));
        next_token();
        return r;
    }

    case tok_float:{
        auto r = Expression::make<AST::Value>(tok.as_float(), m.find("Float"));
        next_token();
        return r;
    }

    case '(': {
        next_token();
        auto r = parse_expression(m, depth + 1);
        EAT(')');
        return r;
    }

    // Function or Variable
    case tok_identifier:{
        auto expr = m.find(tok.identifier());
        next_token();

        // Function
        if (token().type() == '('){
            return parse_function_call(m, expr, depth + 1);
        }
        // TODO: Unary Operator

        // Variable
        return expr;
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

    auto get_op_config = [m](String const& op) -> OpConfig const& {
        return default_precedence()[op];
    };

    // if not a binary op precedence is -1
    while (true){
        tok = token();

        String op;
        if (tok.type() == tok_operator){
            op = tok.operator_name();
        }

        OpConfig const& op_conf = get_op_config(op);
        info("precedence of {} {} < {}", tok.identifier(), op_conf.precedence, precedence);

        if (op_conf.precedence < precedence){
            TRACE_END();
            return lhs;
        }

        tok = next_token();
        auto rhs = parse_primary(m, depth + 1);

        if (!rhs){
            TRACE_END();
            return Expression();
        }

        tok = token();
        String op2;
        if (tok.type() == tok_operator){
            op2 = tok.operator_name();
        }

        OpConfig const& op_conf2 = get_op_config(op2);

        if (op_conf.precedence < op_conf2.precedence){
            rhs = parse_expression_1(m, rhs, op_conf.precedence + 1, depth + 1);
            if (!rhs){
                TRACE_END();
                return Expression();
            }
        }

        lhs = Expression::make<AST::BinaryOperator>(lhs, rhs, get_string(op));
    }
}

void debug_dump(std::ostream& out, Stack<AST::MathNode> const& output_stack){
    auto riter = output_stack.rbegin();

    while (riter != output_stack.rend()) {
        out << (*riter).name << " ";
        ++riter;
    }
    out << "\n";

    auto iter = output_stack.begin();
    while (iter != output_stack.end()) {
        auto op = *iter;
        out << (*iter).name << " ";
        ++iter;
    }
    out << "\n";
}

// return function call
Expression Parser::parse_function_call(Module& m, Expression function, std::size_t depth) {
    TRACE_START();
    auto expr = Expression::make<AST::Call>();
    auto fun = expr.ref<AST::Call>();

    fun->function = function;
    EXPECT('(', "`(` was expected");
    EAT('(');

    while (token().type() != ')') {
        fun->arguments.push_back(parse_expression(m, depth + 1));

        if (token().type() == ',') {
            next_token();
        }
    }

    EXPECT(')', "`)` was expected");
    EAT(')');
    return expr;
}

Expression rpe_function(Parser& self, Module& m, std::size_t depth){
    debug("parse function call");

    Token tok = self.token();

    Expression function;
    int nargs = 0;

    String name = tok.identifier();
    std::tie(function, nargs) = m.find_function(name);

    // eat the identifier
    self.next_token();

    auto expr = self.parse_function_call(m, function, depth + 1);
    return expr;
}

Expression rpe_variable(Parser& self, Module& m){
    Token tok = self.token();
    auto expr = m.find(tok.identifier());

    // eat the value
    self.next_token();
    debug("Added VarRef {} in output stack", tok.identifier().c_str());
    return expr;
}

Expression rpe_value(Parser& self){
    Token tok = self.token();
    self.next_token();
    debug("Added {} in output stack", tok.identifier().c_str());
    return self.make_value(tok);
}
/*
Expression rpe_operator(Parser& self, Module& m, Stack<Expression>& output_stack, Stack<AST::MathNode>& operator_stack){
    String op_name = self.parse_operator();

    if (operator_stack.size() > 0) {
        int current_pred;
        bool current_asso;
        int operator_pred;
        bool is_left_asso;

        //std::tie(current_pred, current_asso) =
        //    m.precedence_table()[op_name];
        auto current = m.precedence_table()[op_name];
        auto op =
        auto node = operator_stack.peek();
        std::tie(operator_pred, is_left_asso) =
            m.precedence_table()[node.name];

        while ((node.kind == AST::MathKind::Function ||
                (operator_pred > current_pred) ||
                (operator_pred == current_pred && is_left_asso)) &&
               (node.name != "(")) {
            // debug("%s is_fun %d | %d > %d | is_left_asso %d",
            // cop.c_str(), is_fun, operator_pred, current_pred,
            // is_left_asso);

            operator_stack.pop();
            debug("pop {} to operator", node.name.c_str());

            output_stack.push(node.ref);
            debug("push {} to output", node.name.c_str());

            if (operator_stack.size() == 0) {
                break;
            }

            node = operator_stack.peek();
            std::tie(operator_pred, is_left_asso) =
                m.precedence_table()[node.name];
        }
    }
    debug("push {} to operator", op_name.c_str());
    return Expression::make<AST::Operator>(op_name);
}
*/

// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
Stack<Expression> parse_rpe(Parser& self, Module& m, std::size_t depth){
    Stack<Expression> output_stack;
    Stack<AST::MathNode> operator_stack;

    while (self.token().type() != tok_newline) {
        Token tok = self.token();
        debug("Token {} {}", tok.type(), to_string(tok.type()));

        switch (tok.type()) {
        case ',': {
            self.next_token();
            continue;
        } // <<<<<<<<<<<<<<--------------------------

        // if a variable or number is found, it is copied directly to the output
        // Value
        case tok_float:
        case tok_int:{
            output_stack.push(rpe_value(self));

//            output_stack.push({
//                AST::MathKind::Value,
//                0,
//                Expression(),
//                tok.identifier()
//            });
            continue;
        } // <<<<<<<<<<<<<<--------------------------

        // function call & variable
        case tok_identifier: {
            if (self.peek_token().type() == '(') {
                operator_stack.push({
                    AST::MathKind::Function,
                    1,
                    rpe_function(self, m, depth)});


//                operator_stack.push({
//                    AST::MathKind::Function,
//                    nargs,
//                    function,
//                    tok.identifier()
//                });
                continue;
            }

            output_stack.push(rpe_variable(self, m));
//            output_stack.push({
//                AST::MathKind::VarRef,
//                0,
//                expr,
//                tok.identifier()
//            });

            continue;
        } // <<<<<<<<<<<<<<--------------------------

        case '(':{
            operator_stack.push({AST::MathKind::None, 0, Expression(), "("});
            debug("push {} to operator", "(");
            self.next_token();
            continue;
        } // <<<<<<<<<<<<<<--------------------------

        case ')':{
            auto node = operator_stack.peek();

            while (node.name != "(" && operator_stack.size() > 0) {
                debug("pop {} to operator {}", node.name, operator_stack.size());
                output_stack.push(operator_stack.pop().ref);
                node = operator_stack.peek();
            }

            if (node.name != "(") {
                warn("mismatched parentheses");
            } else {
                operator_stack.pop();
            }
            self.next_token();
            continue;
        } // <<<<<<<<<<<<<<--------------------------

        // If the symbol is an operator, it is pushed onto the operator stack
        // If the operator's precedence is less than that of the operators at the top of the stack
        // or the precedences are equal and the operator is left associative
        // then that operator is popped off the stack and added to the outpu
        default:{
//            operator_stack.push({
//                AST::MathKind::Operator, 0,
//                rpe_operator(self, m, output_stack, operator_stack)});

            //     operator_stack.push({AST::MathKind::Operator, 0, Expression(), op_name});
            continue;
        } // <<<<<<<<<<<<<<--------------------------

        } // switch
    } // while

    // Finally, any remaining operators are popped off the stack and added to the output
    while (operator_stack.size() > 0) {
        output_stack.push(operator_stack.pop().ref);
    }

    return output_stack;
}

//Expression Parser::parse_expression(Module& m, std::size_t depth) {
//    auto output_stack = parse_rpe(*this, m, depth);

//    // debug_dump(std::cout, output_stack);

//    return Expression::make<AST::ReversePolish>(output_stack);
//}

} // namespace lython
