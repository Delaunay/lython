#include "parser.h"

namespace lython {


StmtNode* not_implemented_stmt(GCObject* parent) {
    return parent->new_object<NotImplementedStmt>();
}

ExprNode* not_implemented_expr(GCObject* parent) {
    return parent->new_object<NotImplementedExpr>();
}

ExprNode* not_allowed_expr(GCObject* parent) {
    return parent->new_object<NotAllowedEpxr>();
}

void Parser::start_code_loc(CommonAttributes* target, Token tok) {}
void Parser::end_code_loc(CommonAttributes* target, Token tok) {}

Token Parser::parse_body(GCObject* parent, Array<StmtNode*>& out, int depth) {
    while (token().type() != tok_desindent && token().type() != tok_eof) {
        out.push_back(parse_statement(parent, depth));
    }

    auto last = token();
    expect_token(tok_desindent, true, parent);
    return last;
}

// Statement_1
StmtNode* Parser::parse_function_def(GCObject* parent, bool async, int depth) {
    FunctionDef* stmt = nullptr;
    auto start = token();
    async_mode.push_back(async);

    if (!async){
        stmt = parent->new_object<FunctionDef>();
    } else {
        next_token(); // eat async
        stmt = parent->new_object<AsyncFunctionDef>();
    }

    start_code_loc(stmt, start);
    next_token();

    stmt->name = get_identifier();
    expect_token(tok_identifier, true, stmt);
    expect_token(tok_parens, true, stmt);
    stmt->args = parse_arguments(stmt, ')', depth + 1);

    if (token().type() == tok_arrow) {
        stmt->returns = parse_expression(stmt, depth + 1);
    }

    expect_token(':', true, stmt);
    expect_token(tok_indent, true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, last);
    async_mode.pop_back();
    return stmt;
}

StmtNode* Parser::parse_class_def(GCObject* parent, int depth) {
    auto stmt = parent->new_object<ClassDef>();
    start_code_loc(stmt, token());
    next_token();

    // Parse bases
    if (token().type() == tok_parens) {
        parse_call_args(stmt, stmt->bases, stmt->keywords, depth + 1);
    }

    expect_token(':', true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_for(GCObject* parent, int depth) {
    For* stmt = nullptr;
    if (!async()){
        stmt = parent->new_object<For>();
    } else {
        stmt = parent->new_object<AsyncFor>();
        stmt->async = true;
    }

    start_code_loc(stmt, token());
    next_token();

    stmt->target = parse_expression(stmt, depth + 1);
    expect_token(tok_in, true, stmt);
    stmt->iter = parse_expression(stmt, depth + 1);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_else) {
        next_token();
        expect_token(tok_indent, true, stmt);
        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_while(GCObject* parent, int depth) {
    auto stmt = parent->new_object<While>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);
        if (token().type() == tok_else) {
        next_token();
        expect_token(tok_indent, true, stmt);
        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_if(GCObject* parent, int depth) {
    auto stmt = parent->new_object<If>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);

    expect_token(':', true, stmt);
    expect_token(tok_indent, true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    // The else belongs to the last ifexpr if any
    if (token().type() == tok_else) {
        next_token();
        expect_token(tok_indent, true, stmt);
        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    // This is interesting looks like python does not chain ifs in the AST
    if (token().type() == tok_elif) {
        auto elseif = parse_if(stmt, depth + 1);
        stmt->orelse.push_back(elseif);
    }

    end_code_loc(stmt, last);
    return stmt;
}

// [<pattern>, <pattern>, ...]
Pattern* Parser::parse_match_sequence(GCObject* parent, int depth) {
    auto pat = parent->new_object<MatchSequence>();
    start_code_loc(pat, token());

    while (token().type() != ']') {
        auto child = parse_pattern(pat, depth + 1);
        pat->patterns.push_back(child);

        if (token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token(']', true, pat);
            break;
        }
    }

    return pat;
}

// *<identifier>
Pattern* Parser::parse_match_star(GCObject* parent, int depth) {
    auto pat = parent->new_object<MatchStar>();
    start_code_loc(pat, token());
    next_token();

    pat->name = get_identifier();
    expect_token(tok_identifier, true, pat);

    end_code_loc(pat, token());
    return pat;
}

// <expr>(<pattern>..., <identifier>=<pattern>)
Pattern* Parser::parse_match_class(GCObject* parent, ExprNode* cls, int depth) {
    auto pat = parent->new_object<MatchClass>();
    pat->cls = cls;

    // TODO: This is the location of '(' not the full pattern
    start_code_loc(pat, token());
    expect_token(tok_parens, true, pat);

    bool keyword = false;
    while (token().type() != ')') {

        // keyword arguments are starting
        if (token().type() == tok_identifier && peek_token().type() == tok_assign) {
            keyword = true;
        }

        // positional arguments
        if (!keyword) {
            pat->patterns.push_back(parse_pattern(pat, depth + 1));
        }

        // keywords
        if (keyword) {
            pat->kwd_attrs.push_back(get_identifier());
            expect_token(tok_identifier, true, pat);
            pat->kwd_patterns.push_back(parse_pattern(pat, depth + 1));
        }

        if (next_token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token(')', true, pat);
            break;
        }
    }

    return pat;
}

// This one does not make much sense
// {'a': Point(x, y)}
// { 1 + 1: 2}
// { <expr>: <pattern> }
Pattern* Parser::parse_match_mapping(GCObject* parent, int depth) {
    auto pat = parent->new_object<MatchMapping>();
    start_code_loc(pat, token());
    next_token();

    while (token().type() != '}') {
        auto key = parse_expression(pat, depth + 1);
        expect_token(':', true, pat);
        auto child = parse_pattern(pat, depth + 1);

        pat->keys.push_back(key);
        pat->patterns.push_back(child);

        if (next_token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token('}', true, pat);
            break;
        }
    }

    return pat;
}

// <pattern> | <pattern> | ...
Pattern* Parser::parse_match_or(GCObject* parent, Pattern* child, int depth) {
    auto pat = parent->new_object<MatchOr>();
    pat->patterns.push_back(child);

    // TODO: this is the loc of '|' not the start of the expression
    start_code_loc(pat, token());
    next_token();

    while (token().type() != ':') {
        child = parse_pattern(pat, depth + 1);
        pat->patterns.push_back(child);

        if (token().type() == '|') {
            next_token();
        } else {
            // could be ":" or "if"
            // expect_token(':', false, pat);
            break;
        }
    }

    end_code_loc(pat, token());
    return pat;
}

// <pattern> as <identifier>
Pattern* Parser::parse_match_as(GCObject* parent, Pattern* primary, int depth) {
    auto pat = parent->new_object<MatchAs>();

    // TODO: this is the loc of 'as' not the start of the expression
    start_code_loc(pat, token());

    expect_token(tok_as, true, pat);
    pat->name = get_identifier();

    end_code_loc(pat, token());
    expect_token(tok_identifier, true, pat);

    return pat;
}

Pattern* Parser::parse_pattern_1(GCObject* parent, int depth) {

    switch(token().type()) {
        case tok_square:     return parse_match_sequence(parent, depth);

        // TODO: make sure those are correct
        case tok_curly:      return parse_match_mapping(parent, depth);
        case tok_star:       return parse_match_star(parent, depth);

        // Constants
        // The values represented can be simple types such as a number, string or None,
        // but also immutable container types (tuples and frozensets)
        // if all of their elements are constant.
        case tok_int:
        case tok_string:
        case tok_float: {
            auto pat = parent->new_object<MatchSingleton>();
            pat->value = parse_constant(pat, depth + 1);
            return pat;
        }

        // case tok_identifier: return parse_match_class(parent, depth);
        // MatchClass is expecting a expression not an identifier
        // this is interesting does that mean if I call a function returning a type
        // it will match on that type ?

        // Value
        default: {
            auto value = parse_expression(parent, depth + 1);
            Pattern* pat = nullptr;

            // <expr> if|:
            if (token().type() != '(') {
                pat = parent->new_object<MatchValue>();
                ((MatchValue*)pat)->value = value;
            } else {
                pat = parse_match_class(parent, value, depth);;
            }

            parent->remove_child(value, false);
            pat->add_child(value);
            return pat;
        }
    }
}

Pattern* Parser::parse_pattern(GCObject* parent, int depth) {
    auto primary = parse_pattern_1(parent, depth);

    switch (token().type()) {
        case '|':    return parse_match_or(parent, primary, depth);
        case tok_as: return parse_match_as(parent, primary, depth);
    }

    // could be ":" or "if"
    // expect_token(':', false, primary);
    return primary;
}

Token Parser::parse_match_case(GCObject* parent, Array<MatchCase>& out, int depth) {
    Token last = token();

    while (token().type() != tok_desindent) {
        MatchCase case_;
        expect_token(tok_case, true, parent);
        // Patern
        case_.pattern = parse_pattern(parent, depth);

        // Guard
        if (token().type() == tok_if) {
            case_.guard = parse_expression(parent, depth + 1);
        }

        expect_token(':', true, parent);
        expect_token(tok_indent, true, parent);

        // Branch
        last = parse_body(parent, case_.body, depth + 1);
        out.push_back(case_);

        // Stop if next token is not for another branch
        if (token().type() != tok_case) {
            expect_token(tok_desindent, false, parent);
            break;
        }
    }

    return last;
}

StmtNode* Parser::parse_match(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Match>();
    start_code_loc(stmt, token());
    next_token();

    stmt->subject = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt);
    expect_token(tok_indent, true, parent);

    auto last = parse_match_case(stmt, stmt->cases, depth);

    end_code_loc(stmt, last);
    return stmt;
}

void Parser::parse_withitem(GCObject* parent, Array<WithItem>& out, int depth) {
    while (token().type() != ':') {
        ExprNode* expr = parse_expression(parent, depth + 1);
        ExprNode* var = nullptr;

        if (token().type() == tok_as) {
            var = parse_expression(parent, depth + 1);
        }

        out.push_back(WithItem{expr, var});

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }
}

StmtNode* Parser::parse_with(GCObject* parent, int depth) {
    With* stmt = nullptr;
    if (!async()){
        stmt = parent->new_object<With>();
    } else {
        stmt = parent->new_object<AsyncWith>();
        stmt->async = true;
    }

    start_code_loc(stmt, token());
    next_token();

    parse_withitem(stmt, stmt->items, depth + 1);

    expect_token(':', true, stmt);
    expect_token(tok_indent, true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    return stmt;
}

StmtNode* Parser::parse_raise(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Raise>();
    start_code_loc(stmt, token());
    next_token();

    if (token().type() != tok_newline)  {
        stmt->exc = parse_expression(stmt, depth + 1);

        if (token().type() == tok_from) {
            next_token();
            stmt->cause = parse_expression(stmt, depth + 1);
        }

        end_code_loc(stmt, token());
    } else {
        end_code_loc(stmt, token());
        next_token();
    }
}

Token Parser::parse_except_handler(GCObject* parent, Array<ExceptHandler>& out, int depth) {
    while (token().type() == tok_except) {
        next_token();
        ExceptHandler handler;

        if (token().type() != ':') {
            handler.type = parse_expression(parent, depth + 1);

            if (token().type() == tok_identifier) {
                handler.name = get_identifier();
                next_token();
            }
        }

        expect_token(':', true, parent);
        parse_body(parent, handler.body, depth + 1);

        out.push_back(handler);
    }
}

StmtNode* Parser::parse_try(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Try>();
    start_code_loc(stmt, token());
    next_token();

    expect_token(':', true, stmt);
    expect_token(tok_indent, true, stmt);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_except){
        parse_except_handler(stmt, stmt->handlers, depth + 1);
    }

    if (token().type() == tok_else) {
        next_token();
        parse_body(stmt, stmt->orelse, depth + 1);
    }

    if (token().type() == tok_finally) {
        next_token();
        parse_body(stmt, stmt->finalbody, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
 }

StmtNode* Parser::parse_assert(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Assert>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);

    if (token().type() == ','){
        next_token();
        stmt->msg = parse_expression(stmt, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
 }

 void Parser::parse_alias(GCObject* parent, Array<Alias>& out, int depth) {
    while (token().type() != tok_newline) {
        Alias alias;
        alias.name = get_identifier();
        expect_token(tok_identifier, true, parent);

        if (token().type() == tok_as) {
            alias.asname = get_identifier();
            expect_token(tok_identifier, true, parent);
        }

        out.push_back(alias);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }
 }

StmtNode* Parser::parse_import(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Import>();
    start_code_loc(stmt, token());
    next_token();

    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    expect_token(tok_newline, true, stmt);
    return stmt;
 }

StmtNode* Parser::parse_import_from(GCObject* parent, int depth) {
    auto stmt = parent->new_object<ImportFrom>();
    start_code_loc(stmt, token());
    next_token();

    // TOOD:
    // count the level ..
    // stmt->level = ;
    stmt->module = get_identifier();
    expect_token(tok_identifier, true, parent);

    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    expect_token(tok_newline, true, stmt);
    return stmt;
 }

StmtNode* Parser::parse_global(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Global>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        stmt->names.push_back(get_identifier());
        expect_token(tok_identifier, true, parent);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(stmt, token());
    expect_token(tok_newline, true, stmt);
    return stmt;
 }

 StmtNode* Parser::parse_nonlocal(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Nonlocal>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        stmt->names.push_back(get_identifier());
        expect_token(tok_identifier, true, parent);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(stmt, token());
    expect_token(tok_newline, true, stmt);
    return stmt;
 }

StmtNode* Parser::parse_return(GCObject* parent, int depth) {
     auto stmt = parent->new_object<Return>();
     start_code_loc(stmt, token());
     next_token();

     if (token().type() != tok_newline)  {
        stmt->value = parse_expression(stmt, depth + 1);
        end_code_loc(stmt, token());
     } else {
         end_code_loc(stmt, token());
         next_token();
     }

     return stmt;
}

StmtNode* Parser::parse_del(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Delete>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        auto expr = parse_expression(stmt, depth + 1);
        stmt->targets.push_back(expr);

        if (token().type() == ',') {
            next_token();
        } else {
            expect_token(tok_newline, true, stmt);
            break;
        }
    }

    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_pass(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Pass>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_break(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Break>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_continue(GCObject* parent, int depth) {
    auto stmt = parent->new_object<Continue>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

// Statement_2
StmtNode* Parser::parse_assign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<Assign>();
    stmt->targets.push_back(expr);

    // FIXME: this is the location of '=' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_augassign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<AugAssign>();
    stmt->target = expr;

    // FIXME: this is the location of the operator not the start of the full expression
    start_code_loc(stmt, token());
    // stmt->op = get_binary_operator(token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);
    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_annassign(GCObject* parent, ExprNode* expr, int depth) {
    auto stmt = parent->new_object<AnnAssign>();
    stmt->target = expr;

    // FIXME: this is the location of :' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->annotation = parse_expression(stmt, depth + 1);

    expect_token(tok_assign, true, stmt);
    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

// parse_expression_1
ExprNode* Parser::parse_name(GCObject* parent, int depth) {
    auto expr = parent->new_object<Name>();
    start_code_loc(expr, token());

    // expr->ctx = ;

    expr->id = get_identifier();
    end_code_loc(expr, token());

    expect_token(tok_identifier, true, expr);
    return expr;
}
ExprNode* Parser::parse_constant(GCObject* parent, int depth){
    auto expr = parent->new_object<Constant>();
    start_code_loc(expr, token());

    switch (token().type()){
    case tok_string:{
        expr->value = token().identifier();
    }
    case tok_int:{
         expr->value = token().as_integer();
    }
    case tok_float:{
        expr->value = token().as_float();
    }
    }

    end_code_loc(expr, token());
    next_token();
    return expr;
}

ExprNode* Parser::parse_await(GCObject* parent, int depth) {
    auto expr = parent->new_object<Await>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_yield(GCObject* parent, int depth){
    auto expr = parent->new_object<Yield>();
    start_code_loc(expr, token());
    next_token();

    if (token().type() != tok_newline)  {
        expr->value = parse_expression(expr, depth + 1);
        end_code_loc(expr, token());
    } else {
        next_token();
        end_code_loc(expr, token());
    }
    return expr;
}

ExprNode* Parser::parse_yield_from(GCObject* parent, int depth){
    auto expr = parent->new_object<YieldFrom>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

Arguments Parser::parse_arguments(GCObject* parent, char kind, int depth) {
    return Arguments();
}

ExprNode* Parser::parse_lambda(GCObject* parent, int depth){
    auto expr = parent->new_object<Lambda>();
    start_code_loc(expr, token());
    next_token();

    expr->args = parse_arguments(expr, ':', depth + 1);
    expr->body = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_joined_string(GCObject* parent, int depth){
    // TODO
    return not_implemented_expr(parent);
}

ExprNode* Parser::parse_ifexp(GCObject* parent, int depth){
    auto expr = parent->new_object<IfExp>();
    start_code_loc(expr, token());

    expr->test = parse_expression(expr, depth + 1);
    expect_token(':', true, expr);
    next_token();

    expr->body = parse_expression(expr, depth + 1);

    expect_token(tok_else, true, expr);
    next_token();

    expr->orelse = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_starred(GCObject* parent, int depth){
    auto expr = parent->new_object<Starred>();
    expect_token(tok_star, true, expr);
    expr->value = parse_expression(expr, depth + 1);
    return expr;
}

template<typename Comp>
ExprNode* parse_comprehension(Parser* parser, GCObject* parent, ExprNode* child, char kind, int depth) {
    auto expr = parent->new_object<Comp>();

    expr->elt = child;
    expr->generators = parser->parse_comprehension(expr, kind, depth);

    parser->end_code_loc(expr, parser->token());
    return expr;
}

ExprNode* parse_dictcomprehension(Parser* parser, GCObject* parent, ExprNode* key, ExprNode* value, char kind, int depth) {
    auto expr = parent->new_object<DictComp>();

    expr->key = key;
    expr->value = value;
    expr->generators = parser->parse_comprehension(expr, kind, depth);

    parser->end_code_loc(expr, parser->token());
    return expr;
}

template<typename Literal>
ExprNode* parse_literal(Parser* parser, GCObject* parent, ExprNode* child, char kind, int depth) {
    // This is a tuple
    auto expr = parent->new_object<Literal>();
    expr->elts.push_back(child);

    // {a, b, c}
    // [a, b, c]
    // (a, b, c)
    while (parser->token().type() != kind) {
        expr->elts.push_back(parser->parse_expression(parent, depth + 1));

        if (parser->token().type() == ',') {
            parser->next_token();
        } else {
            parser->expect_token(kind, true, expr);
            break;
        }
    }

    parser->end_code_loc(expr, parser->token());
    return expr;
}

ExprNode* parse_dictliteral(Parser* parser, GCObject* parent, ExprNode* key, ExprNode* value, char kind, int depth) {
    // This is a tuple
    auto expr = parent->new_object<DictExpr>();
    expr->keys.push_back(key);
    expr->values.push_back(value);

    // {a: b, c: d}
    while (parser->token().type() != kind) {
        expr->keys.push_back(parser->parse_expression(parent, depth + 1));
        parser->expect_token(':', true, expr);
        expr->values.push_back(parser->parse_expression(parent, depth + 1));

        if (parser->token().type() == ',') {
            parser->next_token();
        } else {
            parser->expect_token(kind, true, expr);
            break;
        }
    }

    parser->end_code_loc(expr, parser->token());
    return expr;
}

template<typename Comp, typename Literal>
ExprNode* parse_comprehension_or_literal(Parser* parser, GCObject* parent, int tok, char kind, int depth) {
    // Save the start token to set the code loc when we know if this is a tuple or a generator
    auto start_tok = parser->token();
    auto err = parser->expect_token<ExprNode>(tok, true, nullptr);

    // Warning: the parent is wrong but we need to parse the expression right now
    auto child = parser->parse_expression(parent, depth + 1);
    ExprNode* value = nullptr;

    bool dictionary = true;

    // Dictionary
    if (token.type() == ':') {
        next_token();
        value = parser->parse_expression(parent, depth + 1);
        dictionary = true;
    }
    // ----

    //
    ExprNode* expr = nullptr;
    if (token().type() == tok_for) {
        // This is generator comprehension
        if (dictionary) {
            expr = parse_dictcomprehension(parser, parent, child, value, kind, depth);
        } else {
            expr = parse_comprehension(parser, parent, child, kind, depth);
        }
    } else {
        if (dictionary) {
            expr = parse_dictliteral(parser, parent, child, value, kind, depth);
        } else {
            expr = parse_literal(parser, parent, child, kind, depth);
        }
    }

    // fix the things we could not do at the begining
    add_wip_expr(err, expr);
    parser->start_code_loc(expr, start_tok);
    parent->remove_child(child, false);
    expr->add_child(child);
    // ----------------------------------------------
    return expr;
}

// [a, b] or [a for b in c]
ExprNode* Parser::parse_list(GCObject* parent, int depth) {
    return parse_comprehension_or_literal<ListComp, ListExpr>(this, parent, tok_square, ']', depth);
}

// (a, b) or (a for b in c)
ExprNode* Parser::parse_tuple_generator(GCObject* parent, int depth){
    return parse_comprehension_or_literal<GeneratorExp, TupleExpr>(this, parent, tok_parens, ')', depth);
}

// {a, b} or {a for b in c} or {a: b, c: d} or {a: b for a, b in c}
ExprNode* Parser::parse_set_dict(GCObject* parent, int depth){
    return parse_comprehension_or_literal<SetComp, SetExpr>(this, parent, tok_curly, '}', depth);
}

// parse_expression_2
ExprNode* Parser::parse_named_expr(GCObject* parent, ExprNode* primary, int depth) {
    auto expr = parent->new_object<NamedExpr>();
    expr->target = primary;

    // FIXME: this is the location of ':=' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(parent, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_bool_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_binary_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_compare_operator(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }
ExprNode* Parser::parse_unary(GCObject* parent, ExprNode* primary, int depth){ return not_implemented_expr(parent); }

Token Parser::parse_call_args(GCObject* expr, Array<ExprNode*>& args, Array<Keyword>& keywords, int depth) {
  bool keyword = false;
    while (token().type() != ')') {
        // if not in keyword mode check if next argument is one
        if (keyword == false) {
            auto lookahead = _lex.peek_token();
            if (token().type() == tok_identifier && lookahead.type() == tok_assign) {
                keyword = true;
            }
        }

        if (!keyword) {
            auto arg = parse_expression(expr, depth + 1);
            args.push_back(arg);
        } else {

            auto kwarg = Keyword();
            kwarg.arg = get_identifier(); // <= NB: this checks for tok_identifier
                                          // if not returns a dummy identifier
            expect_token(tok_identifier, true, expr);

            kwarg.value = parse_expression(expr, depth + 1);
            keywords.push_back(kwarg);
        }

        if (token().type() == ',') {
            next_token();
        } else {
            auto last = token();
            expect_token(')', true, expr);
            return last;
        }
    }

    return token();
}

ExprNode* Parser::parse_call(GCObject* parent, ExprNode* primary, int depth){
    auto expr = parent->new_object<Call>();
    start_code_loc(expr, token());
    next_token();

    expr->func = primary;

    // FIXME: this is the location of '(' not the start of the full expression
    start_code_loc(expr, token());
    expect_token(tok_parens, true, expr);

    auto last = parse_call_args(expr, expr->args, expr->keywords, depth + 1);
    end_code_loc(expr, last);
    return expr;
}

ExprNode* Parser::parse_attribute(GCObject* parent, ExprNode* primary, int depth) {
    auto expr = parent->new_object<Attribute>();
    expr->value = primary;

    // FIXME: this is the location of '.' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    expr->attr = get_identifier();

    end_code_loc(expr, token());
    expect_token(tok_identifier, true, expr);

    // expr->ctx = ;
    return expr;
}

ExprNode* Parser::parse_subscript(GCObject* parent, ExprNode* primary, int depth){
    auto expr = parent->new_object<Subscript>();
    expr->value = primary;

    // FIXME: this is the location of '[' not the start of the full expression
    start_code_loc(expr, token());
    expect_token(tok_square, true, expr);

    // a[2:3]       => Slice(2, 3)
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1, 2, 3]   => Tuple(1, 2, 3)
    //
    _allow_slice.push_back(true);

    // We do not allocate the TupleExpr unless required
    Array<ExprNode*> elts;

    while (token().type() != ']') {
        elts.push_back(parse_expression(expr, depth + 1));

        if (token().type() == ',') {
            next_token();
        } else {
            expect_token(']', true, expr);
            break;
        }
    }
    _allow_slice.push_back(false);

    if (elts.size() == 0) {
        errors.push_back(ParsingError::syntax_error("Substript needs at least one argument"));
    }

    if (elts.size() == 1) {
        expr->slice = elts[0];
    }

    if (elts.size() > 1) {
        auto tuple =  expr->new_object<TupleExpr>();
        tuple->elts = elts;
        expr->slice = tuple;
    }

    end_code_loc(expr, token());
    expect_token(']', true, expr);

    return expr;
}

ExprNode* Parser::parse_slice(GCObject* parent, ExprNode* primary, int depth) {
    if (!allow_slice()){
        errors.push_back(ParsingError::syntax_error("Slice is not allowed in this context"));

        // fallback to primary
        return primary;
    }

    auto expr = parent->new_object<Slice>();
    start_code_loc(expr, token());

    expr->lower = primary;
    expect_token(':', true, expr);

    expr->upper = parse_expression(expr, depth + 1);

    if (token().type() == ':') {
        next_token();
        expr->step = parse_expression(expr, depth + 1);
    }
    return expr;
}

}
