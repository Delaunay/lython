#include "parser/parser.h"

namespace lython {

ST::Expr Parser::parse_expression(Module& m, std::size_t depth) {
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
                auto function = m.find(tok.identifier());
                int nargs = 1;
                int loc = m.find_index(tok.identifier());

                // fun
                if (function != nullptr){
                    if (function->kind() == AST::Expression::KindFunction){
                        auto *fun = static_cast<AST::Function *>(function.get());
                        nargs = int(fun->args().size());
                    }
                    else if (function->kind() == AST::Expression::KindBuiltin){
                        auto *fun = static_cast<AST::Builtin *>(function.get());
                        nargs = int(fun->argument_size);
                    }
                }
                else {
                    debug("function %s was not declared",
                          tok.identifier().c_str());
                }

                //m.print(std::cout);

                operator_stack.push({
                    AST::MathKind::Function,
                    nargs,
                    ST::Expr(new AST::Ref(tok.identifier(), loc))
                });

                debug("push %s to operator", tok.identifier().c_str());
                next_token();
                continue;

            } else { // if it is not a function, it is a variable
                int loc = m.find_index(tok.identifier(), true);
                if (loc < 0){
                    warn("Variable (%s) not defined", tok.identifier().c_str());
                }
                output_stack.push({
                   AST::MathKind::VarRef,
                   0,
                   ST::Expr(new AST::Ref(tok.identifier(), loc))
               });

                EAT(tok.type());
                debug("Added VarRef %s in output stack", tok.identifier().c_str());
                continue;
            }

        }
        case tok_float:
        case tok_int:
            output_stack.push({
                AST::MathKind::Value,
                0,
                nullptr,
                tok.identifier()
            });
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
            operator_stack.push({AST::MathKind::Operator, 0, nullptr, op_name});
        }

        if (tok.type() == '(') {
            operator_stack.push({AST::MathKind::None, 0, nullptr, "("});
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

} // namespace lython
