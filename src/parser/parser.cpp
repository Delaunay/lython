#include "parser.h"
#include "utilities/strings.h"

namespace lython {

template <typename T, typename N>
bool in(T const &e, N const &v) {
    return e == v;
}

template <typename T, typename N, typename... Args>
bool in(T const &e, N const &v, Args... args) {
    return e == v || in(e, args...);
}

template <typename T, typename... Args>
bool in(T const &e, Args... args) {
    return in(e, args...);
}

StmtNode *not_implemented_stmt(Node *parent) { return parent->new_object<NotImplementedStmt>(); }

ExprNode *not_implemented_expr(Node *parent) { return parent->new_object<NotImplementedExpr>(); }

ExprNode *not_allowed_expr(Node *parent) { return parent->new_object<NotAllowedEpxr>(); }

void Parser::start_code_loc(CommonAttributes *target, Token tok) {}
void Parser::end_code_loc(CommonAttributes *target, Token tok) {}

Token Parser::parse_body(Node *parent, Array<StmtNode *> &out, int depth) {
    TRACE_START();

    while (token().type() != tok_desindent && token().type() != tok_eof) {
        auto expr = parse_statement(parent, depth + 1);

        if (expr == nullptr) {
            return token();
        }

        out.push_back(expr);

        if (token().type() == tok_incorrect) {
            next_token();
        }

        while (token().type() == tok_newline) {
            next_token();
        }
    }

    auto last = token();
    expect_tokens({tok_desindent, tok_eof}, true, parent, LOC);
    return last;
}

// Statement_1
StmtNode *Parser::parse_function_def(Node *parent, bool async, int depth) {
    TRACE_START();

    FunctionDef *stmt  = nullptr;
    auto         start = token();
    async_mode.push_back(async);

    if (!async) {
        stmt = parent->new_object<FunctionDef>();
    } else {
        next_token(); // eat async
        stmt = parent->new_object<AsyncFunctionDef>();
    }

    start_code_loc(stmt, start);
    next_token();

    stmt->name = get_identifier();
    expect_token(tok_identifier, true, stmt, LOC);
    expect_token(tok_parens, true, stmt, LOC);
    stmt->args = parse_arguments(stmt, ')', depth + 1);

    if (token().type() == tok_arrow) {
        next_token();
        stmt->returns = parse_expression(stmt, depth + 1);
    }

    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    if (token().type() == tok_docstring) {
        stmt->docstring = token().identifier();
        next_token();

        expect_token(tok_newline, true, stmt, LOC);
    }

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, last);
    async_mode.pop_back();

    TRACE_END();
    return stmt;
}

StmtNode *Parser::parse_class_def(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<ClassDef>();
    start_code_loc(stmt, token());
    next_token();

    stmt->name = get_identifier();
    expect_token(tok_identifier, true, stmt, LOC);

    // Parse bases
    if (token().type() == tok_parens) {
        next_token();
        parse_call_args(stmt, stmt->bases, stmt->keywords, depth + 1);
    }

    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    if (token().type() == tok_docstring) {
        stmt->docstring = token().identifier();
        next_token();
    }

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, last);
    return stmt;
}

ExprNode *Parser::parse_star_targets(Node *parent, int depth) {
    // target for for loop
    // star_targets
    return parse_name(parent, depth);

    /*
    star_targets:
        | star_target !','
        | star_target (',' star_target )* [',']
    star_target:
        | '*' (!'*' star_target)
        | target_with_star_atom
    target_with_star_atom:
        | t_primary '.' NAME !t_lookahead
        | t_primary '[' slices ']' !t_lookahead
        | star_atom
    star_atom:
        | NAME
        | '(' target_with_star_atom ')'
        | '(' [star_targets_tuple_seq] ')'
        | '[' [star_targets_list_seq] ']'
    t_lookahead: '(' | '[' | '.'
    */
}

StmtNode *Parser::parse_for(Node *parent, int depth) {
    TRACE_START();

    For *stmt = nullptr;
    if (!async()) {
        stmt = parent->new_object<For>();
    } else {
        stmt        = parent->new_object<AsyncFor>();
        stmt->async = true;
    }

    start_code_loc(stmt, token());
    next_token();

    stmt->target = parse_star_targets(stmt, depth + 1);
    expect_token(tok_in, true, stmt, LOC);
    stmt->iter = parse_expression(stmt, depth + 1);

    expect_token(':', true, parent, LOC);
    expect_token(tok_newline, true, parent, LOC);
    expect_token(tok_indent, true, parent, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_else) {
        next_token();
        expect_token(':', true, stmt, LOC);
        expect_token(tok_newline, true, stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);

        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode *Parser::parse_while(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<While>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_else) {
        next_token();
        expect_token(':', true, stmt, LOC);
        expect_token(tok_newline, true, stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);
        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode *Parser::parse_if(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<If>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);

    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    // The else belongs to the last ifexpr if any
    if (token().type() == tok_else) {
        next_token();

        expect_token(':', true, stmt, LOC);
        expect_token(tok_newline, true, stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);

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
Pattern *Parser::parse_match_sequence(Node *parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchSequence>();
    start_code_loc(pat, token());

    while (token().type() != ']') {
        auto child = parse_pattern(pat, depth + 1);
        pat->patterns.push_back(child);

        if (token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token(']', true, pat, LOC);
            break;
        }
    }

    return pat;
}

// *<identifier>
Pattern *Parser::parse_match_star(Node *parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchStar>();
    start_code_loc(pat, token());
    next_token();

    pat->name = get_identifier();
    expect_token(tok_identifier, true, pat, LOC);

    end_code_loc(pat, token());
    return pat;
}

// <expr>(<pattern>..., <identifier>=<pattern>)
Pattern *Parser::parse_match_class(Node *parent, ExprNode *cls, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchClass>();
    pat->cls = cls;

    // TODO: This is the location of '(' not the full pattern
    start_code_loc(pat, token());
    expect_token(tok_parens, true, pat, LOC);

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
            expect_token(tok_identifier, true, pat, LOC);
            pat->kwd_patterns.push_back(parse_pattern(pat, depth + 1));
        }

        if (next_token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token(')', true, pat, LOC);
            break;
        }
    }

    return pat;
}

// This one does not make much sense
// {'a': Point(x, y)}
// { 1 + 1: 2}
// { <expr>: <pattern> }
Pattern *Parser::parse_match_mapping(Node *parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchMapping>();
    start_code_loc(pat, token());
    next_token();

    while (token().type() != '}') {
        auto key = parse_expression(pat, depth + 1);
        expect_token(':', true, pat, LOC);
        auto child = parse_pattern(pat, depth + 1);

        pat->keys.push_back(key);
        pat->patterns.push_back(child);

        if (next_token().type() == ',') {
            next_token();
        } else {
            end_code_loc(pat, token());
            expect_token('}', true, pat, LOC);
            break;
        }
    }

    return pat;
}

// <pattern> | <pattern> | ...
Pattern *Parser::parse_match_or(Node *parent, Pattern *child, int depth) {
    TRACE_START();

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
            // expect_token(':', false, pat, LOC);
            break;
        }
    }

    end_code_loc(pat, token());
    return pat;
}

// <pattern> as <identifier>
Pattern *Parser::parse_match_as(Node *parent, Pattern *primary, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchAs>();

    // TODO: this is the loc of 'as' not the start of the expression
    start_code_loc(pat, token());

    expect_token(tok_as, true, pat, LOC);
    pat->name = get_identifier();

    end_code_loc(pat, token());
    expect_token(tok_identifier, true, pat, LOC);

    return pat;
}

Pattern *Parser::parse_pattern_1(Node *parent, int depth) {

    switch (token().type()) {
    case tok_square:
        return parse_match_sequence(parent, depth);

    // TODO: make sure those are correct
    case tok_curly:
        return parse_match_mapping(parent, depth);
    case tok_star:
        return parse_match_star(parent, depth);

    // Constants
    // The values represented can be simple types such as a number, string or None,
    // but also immutable container types (tuples and frozensets)
    // if all of their elements are constant.
    case tok_int:
    case tok_string:
    case tok_float: {
        auto pat   = parent->new_object<MatchSingleton>();
        pat->value = get_value();
        return pat;
    }

    // case tok_identifier: return parse_match_class(parent, depth);
    // MatchClass is expecting a expression not an identifier
    // this is interesting does that mean if I call a function returning a type
    // it will match on that type ?

    // Value
    default: {
        auto     value = parse_expression(parent, depth + 1);
        Pattern *pat   = nullptr;

        // <expr> if|:
        if (token().type() != '(') {
            pat                        = parent->new_object<MatchValue>();
            ((MatchValue *)pat)->value = value;
        } else {
            pat = parse_match_class(parent, value, depth);
            ;
        }

        parent->remove_child(value, false);
        pat->add_child(value);
        return pat;
    }
    }
}

Pattern *Parser::parse_pattern(Node *parent, int depth) {
    TRACE_START();

    auto primary = parse_pattern_1(parent, depth);

    switch (token().type()) {
    case '|':
        return parse_match_or(parent, primary, depth);
    case tok_as:
        return parse_match_as(parent, primary, depth);
    }

    // could be ":" or "if"
    // expect_token(':', false, primary, LOC);
    return primary;
}

Token Parser::parse_match_case(Node *parent, Array<MatchCase> &out, int depth) {
    TRACE_START();

    Token last = token();

    while (token().type() != tok_desindent) {
        MatchCase case_;
        expect_token(tok_case, true, parent, LOC);
        // Patern
        case_.pattern = parse_pattern(parent, depth);

        // Guard
        if (token().type() == tok_if) {
            case_.guard = parse_expression(parent, depth + 1);
        }

        expect_token(':', true, parent, LOC);
        expect_token(tok_newline, true, parent, LOC);
        expect_token(tok_indent, true, parent, LOC);

        // Branch
        last = parse_body(parent, case_.body, depth + 1);
        out.push_back(case_);

        // Stop if next token is not for another branch
        if (token().type() != tok_case) {
            expect_token(tok_desindent, false, parent, LOC);
            break;
        }
    }

    return last;
}

StmtNode *Parser::parse_match(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Match>();
    start_code_loc(stmt, token());
    next_token();

    stmt->subject = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, parent, LOC);
    expect_token(tok_indent, true, parent, LOC);

    auto last = parse_match_case(stmt, stmt->cases, depth);

    end_code_loc(stmt, last);
    return stmt;
}

void Parser::parse_withitem(Node *parent, Array<WithItem> &out, int depth) {
    TRACE_START();

    while (token().type() != ':') {
        ExprNode *expr = parse_expression(parent, depth + 1);
        ExprNode *var  = nullptr;

        if (token().type() == tok_as) {
            next_token();
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

StmtNode *Parser::parse_with(Node *parent, int depth) {
    TRACE_START();

    With *stmt = nullptr;
    if (!async()) {
        stmt = parent->new_object<With>();
    } else {
        stmt        = parent->new_object<AsyncWith>();
        stmt->async = true;
    }

    start_code_loc(stmt, token());
    next_token();

    parse_withitem(stmt, stmt->items, depth + 1);

    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    return stmt;
}

StmtNode *Parser::parse_raise(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Raise>();
    start_code_loc(stmt, token());
    next_token();

    if (token().type() != tok_newline) {
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

    return stmt;
}

Token Parser::parse_except_handler(Node *parent, Array<ExceptHandler> &out, int depth) {
    TRACE_START();

    while (token().type() == tok_except) {
        next_token();
        ExceptHandler handler;

        if (token().type() != ':') {
            handler.type = parse_expression(parent, depth + 1);

            if (token().type() == tok_as) {
                next_token();
                handler.name = get_identifier();
                expect_token(tok_identifier, true, parent, LOC);
            }
        }

        expect_token(':', true, parent, LOC);
        expect_token(tok_newline, true, parent, LOC);
        expect_token(tok_indent, true, parent, LOC);

        parse_body(parent, handler.body, depth + 1);

        out.push_back(handler);
    };

    return token();
}

#define SHOW_TOK() error("{}", str(token()));

StmtNode *Parser::parse_try(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Try>();
    start_code_loc(stmt, token());
    next_token();

    expect_token(':', true, stmt, LOC);
    expect_token(tok_newline, true, stmt, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    SHOW_TOK()

    if (token().type() == tok_except) {
        parse_except_handler(stmt, stmt->handlers, depth + 1);
    }

    if (token().type() == tok_else) {
        next_token(); // else
        expect_token(':', true, stmt, LOC);
        expect_token(tok_newline, true, stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);
        parse_body(stmt, stmt->orelse, depth + 1);
    }

    if (token().type() == tok_finally) {
        next_token(); // finally
        expect_token(':', true, stmt, LOC);
        expect_token(tok_newline, true, stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);
        parse_body(stmt, stmt->finalbody, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_assert(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Assert>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);

    if (token().type() == ',') {
        next_token();
        stmt->msg = parse_expression(stmt, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
}

bool is_dot(Token const &tok) {
    return (tok.type() == tok_operator && tok.operator_name() == ".") || tok.type() == tok_dot;
}

String Parser::parse_module_path(Node *parent, int &level, int depth) {
    level = 0;
    Array<String> path;

    while (true) {
        // relative path
        if (is_dot(token()) && path.size() <= 0) {
            level += 1;
            next_token();
        }

        // module name
        if (token().type() == tok_identifier) {
            path.push_back(get_identifier());
            next_token();
        }

        // separator

        if (is_dot(token())) {
            next_token();
        }

        if (token().type() == tok_as || token().type() == ',' || token().type() == tok_newline ||
            token().type() == tok_eof || token().type() == tok_import) {
            break;
        }
    }

    return join(".", path);
}

void Parser::parse_alias(Node *parent, Array<Alias> &out, int depth) {
    TRACE_START();

    while (token().type() != tok_newline && token().type() != tok_eof) {
        Alias alias;

        int level  = 0;
        alias.name = parse_module_path(parent, level, depth);

        if (token().type() == tok_as) {
            alias.asname = get_identifier();
            expect_token(tok_identifier, true, parent, LOC);
        }

        out.push_back(alias);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }
}

StmtNode *Parser::parse_import(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Import>();
    start_code_loc(stmt, token());
    next_token();

    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    expect_tokens({tok_newline, tok_eof}, true, stmt, LOC);
    return stmt;
}

StmtNode *Parser::parse_import_from(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<ImportFrom>();
    start_code_loc(stmt, token());
    next_token();

    // TOOD:
    // count the level ..
    // stmt->level = ;
    stmt->module = get_identifier();
    expect_token(tok_identifier, true, parent, LOC);

    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    expect_tokens({tok_newline, tok_eof}, true, stmt, LOC);
    return stmt;
}

StmtNode *Parser::parse_global(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Global>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        stmt->names.push_back(get_identifier());
        expect_token(tok_identifier, true, parent, LOC);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_nonlocal(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Nonlocal>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        stmt->names.push_back(get_identifier());
        expect_token(tok_identifier, true, parent, LOC);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_return(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Return>();
    start_code_loc(stmt, token());
    next_token();

    if (token().type() != tok_newline) {
        stmt->value = parse_expression(stmt, depth + 1);
        end_code_loc(stmt, token());
    } else {
        end_code_loc(stmt, token());
        next_token();
    }

    TRACE_END();
    return stmt;
}

StmtNode *Parser::parse_del(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Delete>();
    start_code_loc(stmt, token());
    next_token();

    while (token().type() != tok_newline) {
        auto expr = parse_expression(stmt, depth + 1);
        stmt->targets.push_back(expr);

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_pass(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Pass>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode *Parser::parse_break(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Break>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode *Parser::parse_continue(Node *parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Continue>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

// Statement_2
StmtNode *Parser::parse_assign(Node *parent, ExprNode *expr, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Assign>();
    stmt->targets.push_back(expr);

    // FIXME: this is the location of '=' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_augassign(Node *parent, ExprNode *expr, int depth) {
    TRACE_START();

    auto stmt    = parent->new_object<AugAssign>();
    stmt->target = expr;

    // FIXME: this is the location of the operator not the start of the full expression
    start_code_loc(stmt, token());

    auto conf = get_operator_config(token());
    stmt->op  = conf.binarykind;
    next_token();

    stmt->value = parse_expression(stmt, depth + 1);
    end_code_loc(stmt, token());
    return stmt;
}

StmtNode *Parser::parse_annassign(Node *parent, ExprNode *expr, int depth) {
    TRACE_START();

    auto stmt    = parent->new_object<AnnAssign>();
    stmt->target = expr;

    // FIXME: this is the location of :' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->annotation = parse_expression(stmt, depth + 1);

    expect_token(tok_assign, true, stmt, LOC);
    stmt->value = parse_expression(stmt, depth + 1);

    end_code_loc(stmt, token());
    return stmt;
}

// parse_expression_1
ExprNode *Parser::parse_name(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Name>();
    start_code_loc(expr, token());

    // expr->ctx = ;

    expr->id = get_identifier();
    end_code_loc(expr, token());

    expect_token(tok_identifier, true, expr, LOC);
    return expr;
}

ConstantValue Parser::get_value() {
    switch (token().type()) {
    case tok_string: {
        return ConstantValue(token().identifier());
    }
    case tok_int: {
        return ConstantValue(token().as_integer());
    }
    case tok_float: {
        return ConstantValue(token().as_float());
    }
    }

    return ConstantValue();
}

ExprNode *Parser::parse_constant(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Constant>();
    start_code_loc(expr, token());

    expr->value = get_value();

    end_code_loc(expr, token());
    next_token();
    return expr;
}

ExprNode *Parser::parse_await(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Await>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_yield(Node *parent, int depth) {
    TRACE_START();

    if (peek_token().type() == tok_from) {
        return parse_yield_from(parent, depth);
    }

    auto expr = parent->new_object<Yield>();
    start_code_loc(expr, token());
    next_token();

    if (!in(token().type(), tok_newline, tok_eof)) {
        expr->value = parse_expression(expr, depth + 1);
    } else {
        next_token();
    }
    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_yield_from(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<YieldFrom>();
    start_code_loc(expr, token());
    next_token(); // eat yield

    expect_token(tok_from, true, expr, LOC);
    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

bool is_star(Token const &tok) { return tok.type() == tok_operator && tok.operator_name() == "*"; }

bool is_starstar(Token const &tok) {
    return tok.type() == tok_operator && tok.operator_name() == "**";
}

Arguments Parser::parse_arguments(Node *parent, char kind, int depth) {
    TRACE_START();

    Arguments args;

    bool keywords = false;

    while (token().type() != kind) {
        ExprNode *value = nullptr;

        Arg arg;

        bool vararg = false;
        bool kwarg  = false;

        if (is_star(token())) {
            next_token();
            vararg = true;
        }

        if (is_starstar(token())) {
            next_token();
            kwarg = true;
        }

        start_code_loc(&arg, token());
        arg.arg = get_identifier();
        expect_token(tok_identifier, true, parent, LOC);

        if (token().type() == ':' && kind != ':') {
            next_token();
            arg.annotation = parse_expression(parent, depth + 1);
        }

        if (token().type() == tok_assign) {
            next_token();
            keywords = true;
            value    = parse_expression(parent, depth + 1);
        }

        if (vararg) {
            args.vararg = arg;
        } else if (kwarg) {
            args.kwarg = arg;
        } else if (!keywords) {
            args.args.push_back(arg);
            if (value) {
                args.defaults.push_back(value);
            }
            end_code_loc(&arg, token());
        } else {
            args.kwonlyargs.push_back(arg);
            args.kw_defaults.push_back(value);
        }

        if (token().type() == ',') {
            next_token();
        }
    }

    expect_token(kind, true, parent, LOC);
    return args;
}

ExprNode *Parser::parse_lambda(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Lambda>();
    start_code_loc(expr, token());
    next_token();

    expr->args = parse_arguments(expr, ':', depth + 1);
    expr->body = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_joined_string(Node *parent, int depth) {
    TRACE_START();

    // TODO
    return not_implemented_expr(parent);
}

ExprNode *Parser::parse_ifexp(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<IfExp>();
    start_code_loc(expr, token());
    next_token();

    expr->test = parse_expression(expr, depth + 1);
    expect_token(':', true, expr, LOC);

    expr->body = parse_expression(expr, depth + 1);

    expect_token(tok_else, true, expr, LOC);

    expr->orelse = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_starred(Node *parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Starred>();
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    return expr;
}

void Parser::parse_comprehension(Node *parent, Array<Comprehension> &out, char kind, int depth) {
    TRACE_START();

    while (token().type() != kind) {
        expect_token(tok_for, true, parent, LOC);
        Comprehension cmp;

        cmp.target = parse_star_targets(parent, depth + 1);
        expect_token(tok_in, true, parent, LOC);
        cmp.iter = parse_expression(parent, depth + 1);

        while (token().type() == tok_if) {
            next_token();
            cmp.ifs.push_back(parse_expression(parent, depth + 1));
        }

        cmp.is_async = async();
        out.push_back(cmp);
    }
}

template <typename Comp>
ExprNode *parse_comprehension(Parser *parser, Node *parent, ExprNode *child, char kind, int depth) {
    auto expr = parent->new_object<Comp>();

    expr->elt = child;
    parser->parse_comprehension(expr, expr->generators, kind, depth);

    parser->end_code_loc(expr, parser->token());
    return expr;
}

ExprNode *parse_dictcomprehension(Parser *parser, Node *parent, ExprNode *key, ExprNode *value,
                                  char kind, int depth) {
    auto expr = parent->new_object<DictComp>();

    expr->key   = key;
    expr->value = value;
    parser->parse_comprehension(expr, expr->generators, kind, depth);

    parser->end_code_loc(expr, parser->token());
    return expr;
}

template <typename Literal>
ExprNode *parse_literal(Parser *parser, Node *parent, ExprNode *child, char kind, int depth) {
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
            parser->expect_token(kind, true, expr, LOC);
            break;
        }
    }

    parser->end_code_loc(expr, parser->token());
    return expr;
}

ExprNode *parse_dictliteral(Parser *parser, Node *parent, ExprNode *key, ExprNode *value, char kind,
                            int depth) {
    // This is a tuple
    auto expr = parent->new_object<DictExpr>();
    expr->keys.push_back(key);
    expr->values.push_back(value);

    // {a: b, c: d}
    while (parser->token().type() != kind) {
        expr->keys.push_back(parser->parse_expression(parent, depth + 1));
        parser->expect_token(':', true, expr, LOC);
        expr->values.push_back(parser->parse_expression(parent, depth + 1));

        if (parser->token().type() == ',') {
            parser->next_token();
        } else {
            parser->expect_token(kind, true, expr, LOC);
            break;
        }
    }

    parser->end_code_loc(expr, parser->token());
    return expr;
}

template <typename Comp, typename Literal>
ExprNode *parse_comprehension_or_literal(Parser *parser, Node *parent, int tok, char kind,
                                         int depth) {
    // Save the start token to set the code loc when we know if this is a tuple or a generator
    auto start_tok = parser->token();
    auto err       = parser->expect_token(tok, true, nullptr, LOC); // eat (  [  {

    // Warning: the parent is wrong but we need to parse the expression right now
    auto      child = parser->parse_expression(parent, depth + 1);
    ExprNode *value = nullptr;

    bool dictionary = false;

    // Dictionary
    if (parser->token().type() == ':') {
        parser->next_token();
        value      = parser->parse_expression(parent, depth + 1);
        dictionary = true;
    }
    // ----

    //
    ExprNode *expr = nullptr;
    if (parser->token().type() == tok_for) {
        // This is generator comprehension
        if (dictionary) {
            expr = parse_dictcomprehension(parser, parent, child, value, kind, depth);
        } else {
            expr = parse_comprehension<Comp>(parser, parent, child, kind, depth);
        }
    } else if (parser->token().type() == ',') {
        parser->next_token();

        if (dictionary) {
            expr = parse_dictliteral(parser, parent, child, value, kind, depth);
        } else {
            expr = parse_literal<Literal>(parser, parent, child, kind, depth);
        }
    } else {
        // This is not a literal nor a list-comprehension
        // (2 + 1)
        if (kind == ')' && !dictionary) {
            auto p = parser->parse_expression_1(parent, child, 0, depth);
            parser->expect_token(')', true, parent, LOC);
            return p;
        }

        error("Unhandled list-comprehension case");
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
ExprNode *Parser::parse_list(Node *parent, int depth) {
    TRACE_START();
    return parse_comprehension_or_literal<ListComp, ListExpr>(this, parent, tok_square, ']',
                                                              depth + 1);
}

// (a, b) or (a for b in c)
ExprNode *Parser::parse_tuple_generator(Node *parent, int depth) {
    TRACE_START();
    return parse_comprehension_or_literal<GeneratorExp, TupleExpr>(this, parent, tok_parens, ')',
                                                                   depth + 1);
}

// {a, b} or {a for b in c} or {a: b, c: d} or {a: b for a, b in c}
ExprNode *Parser::parse_set_dict(Node *parent, int depth) {
    TRACE_START();
    return parse_comprehension_or_literal<SetComp, SetExpr>(this, parent, tok_curly, '}', depth);
}

// parse_expression_2
ExprNode *Parser::parse_named_expr(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();

    auto expr    = parent->new_object<NamedExpr>();
    expr->target = primary;

    // FIXME: this is the location of ':=' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(parent, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_bool_operator(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode *Parser::parse_binary_operator(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode *Parser::parse_compare_operator(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode *Parser::parse_suffix_unary(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}

ExprNode *Parser::parse_prefix_unary(Node *parent, int depth) {
    TRACE_START();
    auto expr = parent->new_object<UnaryOp>();
    start_code_loc(expr, token());

    auto conf = get_operator_config(token());

    if (token().operator_name() == "*") {
        return parse_starred(parent, depth);
    }

    if (conf.unarykind == UnaryOperator::None) {
        error("expected an unary operator not {}", str(token()));
    }

    next_token();

    expr->op      = conf.unarykind;
    expr->operand = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

Token Parser::parse_call_args(Node *expr, Array<ExprNode *> &args, Array<Keyword> &keywords,
                              int depth) {
    TRACE_START();

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
            kwarg.arg  = get_identifier(); // <= NB: this checks for tok_identifier
                                           // if not returns a dummy identifier
            expect_token(tok_identifier, true, expr, LOC);
            expect_token(tok_assign, true, expr, LOC);

            kwarg.value = parse_expression(expr, depth + 1);
            keywords.push_back(kwarg);
        }

        if (token().type() == ',') {
            next_token();
        } else {
            auto last = token();
            expect_token(')', true, expr, LOC);
            return last;
        }
    }

    return token();
}

ExprNode *Parser::parse_call(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Call>();
    start_code_loc(expr, token());
    expr->func = primary;

    // FIXME: this is the location of '(' not the start of the full expression
    start_code_loc(expr, token());
    expect_token(tok_parens, true, expr, LOC);

    auto last = parse_call_args(expr, expr->args, expr->keywords, depth + 1);
    end_code_loc(expr, last);

    TRACE_END();
    return expr;
}

ExprNode *Parser::parse_attribute(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();

    auto expr   = parent->new_object<Attribute>();
    expr->value = primary;

    // FIXME: this is the location of '.' not the start of the full expression
    start_code_loc(expr, token());
    next_token();

    expr->attr = get_identifier();

    end_code_loc(expr, token());
    expect_token(tok_identifier, true, expr, LOC);

    // expr->ctx = ;
    return expr;
}

ExprNode *Parser::parse_subscript(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();

    auto expr   = parent->new_object<Subscript>();
    expr->value = primary;

    // FIXME: this is the location of '[' not the start of the full expression
    start_code_loc(expr, token());
    expect_token(tok_square, true, expr, LOC);

    // a[2:3]       => Slice(2, 3)
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1, 2, 3]   => Tuple(1, 2, 3)
    //
    _allow_slice.push_back(true);

    // We do not allocate the TupleExpr unless required
    Array<ExprNode *> elts;

    while (token().type() != ']') {
        elts.push_back(parse_expression(expr, depth + 1));

        if (token().type() == ',') {
            next_token();
        } else {
            expect_token(']', true, expr, LOC);
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
        auto tuple  = expr->new_object<TupleExpr>();
        tuple->elts = elts;
        expr->slice = tuple;
    }

    end_code_loc(expr, token());
    return expr;
}

ExprNode *Parser::parse_slice(Node *parent, ExprNode *primary, int depth) {
    TRACE_START();

    if (!allow_slice()) {
        errors.push_back(ParsingError::syntax_error("Slice is not allowed in this context"));

        // fallback to primary
        return primary;
    }

    auto expr = parent->new_object<Slice>();
    start_code_loc(expr, token());

    expr->lower = primary;
    expect_token(':', true, expr, LOC);

    expr->upper = parse_expression(expr, depth + 1);

    if (token().type() == ':') {
        next_token();
        expr->step = parse_expression(expr, depth + 1);
    }
    return expr;
}

ExprNode *Parser::parse_operators(Node *parent, ExprNode *lhs, int min_precedence, int depth) {
    TRACE_START();

    while (true) {
        auto lookahead = token();
        auto op_conf   = get_operator_config(lookahead);
        auto oppred    = op_conf.precedence;

        // lookahead is a binary operator whose precedence is >= min_precedence
        if (!(is_binary_operator_family(op_conf) && oppred >= min_precedence)) {
            break;
        }

        next_token();
        auto rhs  = parse_expression(parent, depth);
        lookahead = token();

        auto lookconf    = get_operator_config(lookahead);
        auto lookpred    = lookconf.precedence;
        auto right_assoc = !lookconf.left_associative;

        // lookahead is a binary operator whose precedence is greater
        // than op's, or a right-associative operator
        // whose precedence is equal to op's
        while (is_binary_operator_family(op_conf) &&
               (lookpred > oppred || (right_assoc && lookpred == oppred))) {
            rhs       = parse_expression_1(parent, rhs, oppred + 1, depth + 1);
            lookahead = token();
        }

        // the result of applying op with operands lhs and rhs
        if (op_conf.binarykind != BinaryOperator::None) {
            auto result   = parent->new_object<BinOp>();
            result->left  = lhs;
            result->op    = op_conf.binarykind;
            result->right = rhs;
            lhs           = result;
        } else if (op_conf.cmpkind != CmpOperator::None) {
            auto result  = parent->new_object<Compare>();
            result->left = lhs;
            result->ops.push_back(op_conf.cmpkind);
            result->comparators.push_back(rhs);
            lhs = result;
        } else if (op_conf.boolkind != BoolOperator::None) {
            // TODO: check why is this not a binary node ?
            auto result    = parent->new_object<BoolOp>();
            result->op     = op_conf.boolkind;
            result->values = {lhs, rhs};
            lhs            = result;
        } else {
            error("unknow operator {}", str(op_conf));
        }
    }

    TRACE_END();
    return lhs;
}

} // namespace lython
