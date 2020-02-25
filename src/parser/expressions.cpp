#include "parser/parser.h"

namespace lython {

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

void rpe_function(Parser& self, Module& m, Stack<AST::MathNode>& operator_stack){
    Token tok = self.token();

    Expression function;
    int nargs = 0;

    std::tie(function, nargs) = m.find_function(tok.identifier());

    operator_stack.push({
        AST::MathKind::Function,
        nargs,
        function
    });

    debug("push {} to operator", tok.identifier().c_str());
    self.next_token();
}

void rpe_variable(Parser& self, Module& m, Stack<AST::MathNode>& output_stack){
    Token tok = self.token();

    int loc = m.find_index(tok.identifier(), true);
    auto expr = m.find(tok.identifier());

    if (loc < 0){
        warn("Variable ({}) not defined", tok.identifier().c_str());
    }

    output_stack.push({
        AST::MathKind::VarRef,
        0,
        expr
    });

    self.next_token();
    debug("Added VarRef {} in output stack", tok.identifier().c_str());
}

void rpe_value(Parser& self, Stack<AST::MathNode>& output_stack){
    Token tok = self.token();

    output_stack.push({
        AST::MathKind::Value,
        0,
        Expression(),
        tok.identifier()
    });

    self.next_token();
    debug("Added {} in output stack", tok.identifier().c_str());
}

void rpe_operator(Parser& self, Module& m, Stack<AST::MathNode>& output_stack, Stack<AST::MathNode>& operator_stack){
    String op_name = self.parse_operator();

    if (operator_stack.size() > 0) {
        int current_pred;
        bool current_asso;
        int operator_pred;
        bool is_left_asso;

        std::tie(current_pred, current_asso) =
            m.precedence_table()[op_name];

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

            output_stack.push(node);
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
    operator_stack.push({AST::MathKind::Operator, 0, Expression(), op_name});
}

// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
Expression Parser::parse_expression(Module& m, std::size_t depth) {
    TRACE_START();
    Stack<AST::MathNode> output_stack;
    Stack<AST::MathNode> operator_stack;

    while (token().type() != tok_newline) {
        Token tok = token();
        if (tok.type() == ',') {
            tok = next_token();
        }

        // if a variable or number is found, it is copied directly to the
        // output
        switch (tok.type()) {

        // function call & variable
        case tok_identifier: {
            if (peek_token().type() == '(') {
                rpe_function(*this, m, operator_stack);
                continue;
            }

            rpe_variable(*this, m, output_stack);
            continue;
        } // case identifier

        // Value
        case tok_float:
        case tok_int:{
            rpe_value(*this, output_stack);
            continue;
        }

        case '(':{
            operator_stack.push({AST::MathKind::None, 0, Expression(), "("});
            debug("push {} to operator", "(");
            next_token();
            continue;
        }

        case ')':{
            auto node = operator_stack.peek();

            while (node.name != "(" && operator_stack.size() > 0) {
                output_stack.push(operator_stack.pop());
                node = operator_stack.peek();
            }

            if (node.name != "(") {
                warn("mismatched parentheses");
            } else {
                operator_stack.pop();
            }
            next_token();
            continue;
        }

        // If the symbol is an operator, it is pushed onto the operator stack
        // If the operator's precedence is less than that of the operators at the top of the stack
        // or the precedences are equal and the operator is left associative
        // then that operator is popped off the stack and added to the outpu
        default:{
            rpe_operator(*this, m, output_stack, operator_stack);
            continue;
        } // default
        } // switch
    } // while

    // Finally, any remaining operators are popped off the stack and added to the output
    while (operator_stack.size() > 0) {
        output_stack.push(operator_stack.pop());
    }

    return Expression::make<AST::ReversePolish>(output_stack);
}

} // namespace lython
