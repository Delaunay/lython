#include "parser/parser.h"
#include "utilities/printing.h"
#include "utilities/guard.h"
#include "utilities/strings.h"

#define TRACE_START2(tok) \
    kwtrace_start((this->parsinglog),         \
        depth, "{}: {} - `{}`", to_string(tok.type()).c_str(), tok.type(), tok.identifier())

#define TRACE_END2(tok) kwtrace_end((this->parsinglog), depth, "{}: {}", to_string(tok.type()).c_str(), tok.type())

#define TRACE_START() TRACE_START2(token())

#define TRACE_END() TRACE_END2(token())

#define MAYBE_COMMENT(obj, attr)                        \
    if (token().type() == tok_comment) {                \
        (obj)->attr = parse_comment(parent, depth + 1); \
    }

namespace lython {

#define SHOW_TOK(parser, tok) kwerror((parser->parsinglog), "{}", str(tok));

// Reduce the number of dynamic alloc
// but debug info is not kept for the nodes
#define AVOID_DUPLICATE_CONST 0

template <typename Literal>
ExprNode* parse_literal(Parser* parser, Node* parent, ExprNode* child, char kind, int depth);

StmtNode* not_implemented_stmt(Node* parent) { return parent->new_object<NotImplementedStmt>(); }

ExprNode* not_implemented_expr(Node* parent) { return parent->new_object<NotImplementedExpr>(); }

ExprNode* not_allowed_expr(Node* parent) { return parent->new_object<NotAllowedEpxr>(); }

void Parser::start_code_loc(CommonAttributes* target, Token tok) {
    target->col_offset = tok.begin_col();
    target->lineno     = tok.line();
}
void Parser::end_code_loc(CommonAttributes* target, Token tok) {
    target->col_offset = tok.end_col();
    target->end_lineno = tok.line();
}

#define PARSER_THROW(T, err) throw T(err.message)

// Helpers
// ---------------------------------------------

void Parser::ensure_valid() {
    if (has_errors()) {
        throw ParsingException();
    }
}

void Parser::show_diagnostics(std::ostream& out) {
    //
    if (has_errors()) {
        out << "Parsing error messages (" << errors.size() << ")\n";

        ParsingErrorPrinter printer(out, &_lex);
        printer.with_compiler_code_loc = true;
        printer.indent                 = 1;

        for (ParsingError const& error: errors) {
            out << "  ";
            printer.print(error);
            out << "\n";
        }
    }
}

void Parser::expect_token(int expected, bool eat, Node* wip_expression, CodeLocation const& loc) {
    return expect_tokens(Array<int>{expected}, eat, wip_expression, loc);
}

OpConfig const& Parser::get_operator_config(Token const& tok) const {
    static OpConfig nothing;

    Dict<String, OpConfig> const& confs = default_precedence();

    auto result = confs.find(tok.operator_name());
    if (result == confs.end()) {
        return nothing;
    }
    return result->second;
}

bool Parser::is_binary_operator_family(OpConfig const& conf) {
    return conf.binarykind != BinaryOperator::None || conf.cmpkind != CmpOperator::None ||
           conf.boolkind != BoolOperator::None;
}

void Parser::expect_operator(String const&       op,
                             bool                eat,
                             Node*               wip_expression,
                             CodeLocation const& loc) {
    Token tok = token();

    expect_token(tok_operator, eat, wip_expression, LOC);

    if (tok.operator_name() == op) {
        return;
    }

    ParsingError& error = parser_kwerror(                                         //
        loc,                                                                      //
        "SyntaxError",                                                            //
        fmtstr("Wrong operator expected {} but got {}", op, tok.operator_name())  //
                                                                                  //
    );

    add_wip_expr(error, wip_expression);
    PARSER_THROW(SyntaxError, error);
}

void Parser::expect_tokens(Array<int> const&   expected,
                           bool                eat,
                           Node*               wip_expression,
                           CodeLocation const& loc) {
    auto toktype = token().type();

    for (auto& tok: expected) {
        if (toktype == tok) {
            if (eat) {
                next_token();
            }
            return;
        }
    }
    // ----
    Array<String> expected_str;
    expected_str.reserve(expected.size());
    for (auto& ex: expected) {
        expected_str.push_back(str(TokenType(ex)));
    }

    ParsingError& error   = parser_kwerror(                            //
        loc,                                                         //
        "SyntaxError",                                               //
        fmtstr("Expected {} got {}", join(", ", expected), toktype)  //
    );
    error.expected_tokens = expected;
    error.received_token  = token();

    add_wip_expr(error, wip_expression);
    PARSER_THROW(SyntaxError, error);
}


StmtNode* Parser::parse_one(Node* parent, int depth, bool interactive) {
    TRACE_START();

    Token tok = token();
    while (in(tok.type(), tok_newline)) {
        tok = next_token();
    }

    if (in(token().type(), tok_desindent, tok_eof)){
        return nullptr;
    }

    // Found an unexpected token
    // eat the full line to try to recover and emit an error
    if (token().type() == tok_incorrect) {
        ParsingError& error = parser_kwerror(  //
            LOC,                               //
            "SyntaxError",                     //
            "Unexpected token"                 //
        );
        add_wip_expr(error, parent);
        error_recovery(&error);

        InvalidStatement* stmt = parent->new_object<InvalidStatement>();
        stmt->tokens           = error.line;
        return stmt;
        // out.push_back(stmt);
        // continue;
    }

    // Comment attach themselves to the next statement
    // when comments are inserted at the beginning of a block
    // they can be inserted to the previous block instead
    if (token().type() == tok_comment) {
        StmtNode* cmt = parse_comment_stmt(parent, depth);
        _pending_comments.push_back(cmt);
        return parse_one(parent, depth);

        // _pending_comments.push_back(cmt);
        // continue;
    }

    // we have read a bunch of comments and we are still in this block
    if (_pending_comments.size() > 0) {
        // for (auto* comment: _pending_comments) {
        //     out.push_back(comment);
        // }
        // _pending_comments.clear();

        auto element = _pending_comments.front();
        _pending_comments.erase(_pending_comments.begin());
        return element;
    }

    try {
        auto stmt = parse_statement(parent, depth + 1);

        if (interactive) {
            return stmt;
        }

        // only one liner should have the comment attached
        if (stmt->is_one_line() && token().type() == tok_comment) {
            stmt->comment = parse_comment(stmt, depth);
        }

        if (!is_empty_line) {
            // expects at least one newline to end the statement
            // if not we do not know what this line is supposed to be
            expect_tokens({tok_newline, tok_eof}, true, parent, LOC);
        }

        if (stmt == nullptr) {
            // return token();
            return nullptr;
        }

        // out.push_back(stmt);
        return stmt;
    } catch (ParsingException const&) {
        //
        ParsingError* error = &errors[current_error];
        error_recovery(error);

        InvalidStatement* stmt = parent->new_object<InvalidStatement>();
        stmt->tokens           = error->line;
        return stmt;
        // out.push_back(stmt);
    }

    // look for the desindent token or next statement
    while (token().type() == tok_newline) {
        next_token();
    }

    // push comments here ??

    TRACE_END();
    return nullptr;
}

Token Parser::parse_body(Node* parent, Array<StmtNode*>& out, int depth) {
    TRACE_START();

    while (!in(token().type(), tok_desindent, tok_eof)) {
        
        if (StmtNode* stmt = parse_one(parent, depth)) {
            out.push_back(stmt);
            continue;
        }
    }

    if (out.size() <= 0) {
        ParsingError& error = parser_kwerror(  //
            LOC,                               //
            "SyntaxError",                     //
            "Expected a body"                  //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    }

    auto last = token();

    if (last.type() == tok_eof && depth == 0) {
        // reached eof, insert all the comments here
        for (auto* comment: _pending_comments) {
            out.push_back(comment);
        }
        _pending_comments.clear();
    }
    //
    expect_tokens({tok_desindent, tok_eof}, true, parent, LOC);
    return last;
}

bool Parser::is_tok_statement_ender() const {
    // returns true if the token terminates a statement
    return in(token().type(), tok_newline, tok_eof, tok_comment);
}

void Parser::expect_newline(Node* stmt, CodeLocation const& loc) {
    expect_token(tok_newline, true, stmt, loc);

    while (token().type() == tok_newline) {
        next_token();
    }
}

// Statement_1
StmtNode* Parser::parse_function_def(Node* parent, bool async, int depth) {
    TRACE_START();

    FunctionDef* stmt  = nullptr;
    auto         start = token();
    async_mode.push_back(async);

    if (!async) {
        stmt = parent->new_object<FunctionDef>();
    } else {
        next_token();  // eat async
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
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    if (token().type() == tok_docstring) {
        Comment* comment   = nullptr;
        String   docstring = token().identifier();

        next_token();
        if (token().type() == tok_comment) {
            comment = parse_comment(parent, depth + 1);
        }

        stmt->docstring = Docstring(docstring, comment);
        expect_newline(stmt, LOC);
    }

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, last);
    async_mode.pop_back();

    TRACE_END();
    return stmt;
}

StmtNode* Parser::parse_class_def(Node* parent, int depth) {
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
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    if (token().type() == tok_docstring) {
        Comment* comment   = nullptr;
        String   docstring = token().identifier();
        next_token();

        if (token().type() == tok_comment) {
            comment = parse_comment(parent, depth + 1);
        }

        stmt->docstring = Docstring(docstring, comment);
        expect_newline(stmt, LOC);
    }

    auto last = parse_body(stmt, stmt->body, depth + 1);

    for (auto child: stmt->body) {
        // this checks we do not have expression inside the body of a class
        Expr* exprstmt = cast<Expr>(child);

        // Comments are fine though
        if (exprstmt && exprstmt->value->kind != NodeKind::Comment) {
            ParsingError& error = parser_kwerror(          //
                LOC,                                       //
                "SyntaxError",                             //
                "Unsupported statement inside a classdef"  //
            );
            add_wip_expr(error, parent);
            PARSER_THROW(SyntaxError, error);
        }
    }

    end_code_loc(stmt, last);
    return stmt;
}
ExprNode* Parser::parse_star_expression(Node* parent, int depth) {
    // star_expressions:
    //     | star_expression (',' star_expression )+ [',']
    //     | star_expression ','
    //     | star_expression
    // star_expression:
    //     | '*' bitwise_or
    //     | expression
    return parse_expression(parent, depth);
}

ExprNode* Parser::parse_star_targets(Node* parent, int depth) {
    auto start_tok = token();

    // auto kind      = 0;
    // bool has_parens = false;
    if (token().type() == tok_parens) {
        next_token();
        // kind = tok_parens;
        // has_parens = true;
    }

    Array<ExprNode*> elts;
    auto             r = parse_expression_primary(parent, depth + 1);
    elts.push_back(r);

    while (token().type() == tok_comma) {
        next_token();
        elts.push_back(parse_expression_primary(parent, depth + 1));
    };

    if (elts.size() == 1) {
        start_code_loc(elts[0], start_tok);
        end_code_loc(elts[0], token());
        return elts[0];
    }

    auto expr = parent->new_object<TupleExpr>();
    start_code_loc(expr, start_tok);
    expr->elts = elts;
    expr->ctx  = ExprContext::Store;
    end_code_loc(expr, token());
    return expr;

    // star_targets:
    //     | star_target !','
    //     | star_target (',' star_target )* [',']
    // star_target:
    //     | '*' (!'*' star_target)
    //     | target_with_star_atom
    // target_with_star_atom:
    //     | t_primary '.' NAME !t_lookahead
    //     | t_primary '[' slices ']' !t_lookahead
    //     | star_atom
    // star_atom:
    //     | NAME
    //     | '(' target_with_star_atom ')'
    //     | '(' [star_targets_tuple_seq] ')'
    //     | '[' [star_targets_list_seq] ']'
    // t_primary:
    //     | t_primary '.' NAME &t_lookahead
    //     | t_primary '[' slices ']' &t_lookahead
    //     | t_primary genexp &t_lookahead
    //     | t_primary '(' [arguments] ')' &t_lookahead
    //     | atom &t_lookahead
    // t_lookahead: '(' | '[' | '.'
}

void Parser::expect_comment_or_newline(StmtNode* stmt, int depth, CodeLocation const& loc) {
    add_inline_comment(stmt, depth);
    expect_newline(stmt, loc);
}

StmtNode* Parser::parse_for(Node* parent, int depth) {
    TRACE_START();

    For* stmt = nullptr;
    if (!async()) {
        stmt = parent->new_object<For>();
    } else {
        stmt        = parent->new_object<AsyncFor>();
        stmt->async = true;
    }

    start_code_loc(stmt, token());
    next_token();

    // Store context i.e the variables are created there
    _context.push_back(ExprContext::Store);
    stmt->target = parse_star_targets(stmt, depth + 1);
    _context.pop_back();

    // the right context was already picked up
    // set_context(stmt->target, ExprContext::Store);

    expect_token(tok_in, true, stmt, LOC);
    stmt->iter = parse_expression(stmt, depth + 1);

    expect_token(':', true, parent, LOC);

    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, parent, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_else) {
        next_token();
        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, else_comment);
        expect_newline(stmt, LOC);

        expect_token(tok_indent, true, stmt, LOC);

        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_while(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<While>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt, LOC);
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    if (token().type() == tok_else) {
        next_token();
        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, else_comment);
        expect_newline(stmt, LOC);

        expect_token(tok_indent, true, stmt, LOC);
        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_if_alt(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<If>();
    start_code_loc(stmt, token());
    next_token();
    Token last = dummy();

    // Parse first If
    {
        stmt->test = parse_expression(stmt, depth + 1);

        expect_token(':', true, stmt, LOC);
        expect_comment_or_newline(stmt, depth, LOC);
        expect_token(tok_indent, true, stmt, LOC);

        auto last = parse_body(stmt, stmt->body, depth + 1);
    }

    while (token().type() == tok_elif) {
        next_token();

        auto test = parse_expression(stmt, depth + 1);
        expect_token(':', true, stmt, LOC);

        // We need to push a comment even if there is nothing
        // because we will zip between tests and tests_comments
        int n = int(stmt->tests_comment.size());
        stmt->tests_comment.push_back(nullptr);

        MAYBE_COMMENT(stmt, tests_comment[n]);
        expect_newline(stmt, LOC);
        expect_token(tok_indent, true, stmt, LOC);

        Array<StmtNode*> body;
        last = parse_body(stmt, body, depth + 1);

        stmt->tests.push_back(test);
        stmt->bodies.push_back(body);
    }

    // The else belongs to the last ifexpr if any
    if (token().type() == tok_else) {
        next_token();

        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, else_comment);
        expect_newline(stmt, LOC);

        expect_token(tok_indent, true, stmt, LOC);

        last = parse_body(stmt, stmt->orelse, depth + 1);
    }

    end_code_loc(stmt, last);
    return stmt;
}

StmtNode* Parser::parse_if(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<If>();
    start_code_loc(stmt, token());
    next_token();

    stmt->test = parse_expression(stmt, depth + 1);

    expect_token(':', true, stmt, LOC);
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    // The else belongs to the last if if any
    if (token().type() == tok_else) {
        next_token();

        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, else_comment);
        expect_newline(stmt, LOC);

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
Pattern* Parser::parse_match_sequence(Node* parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchSequence>();
    start_code_loc(pat, token());
    next_token();

    while (token().type() != ']') {
        auto child = parse_pattern(pat, depth + 1);
        pat->patterns.push_back(child);

        if (token().type() == tok_comma) {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(pat, token());
    expect_token(']', true, pat, LOC);
    TRACE_END();
    return pat;
}

// *<identifier>
Pattern* Parser::parse_match_star(Node* parent, int depth) {
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
Pattern* Parser::parse_match_class(Node* parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchClass>();

    start_code_loc(pat, token());
    pat->cls = parse_expression_primary(pat, depth);

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
        else if (keyword) {
            pat->kwd_attrs.push_back(get_identifier());
            expect_token(tok_identifier, true, pat, LOC);
            expect_token(tok_assign, true, pat, LOC);
            pat->kwd_patterns.push_back(parse_pattern(pat, depth + 1));
        }

        if (token().type() == tok_comma) {
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
Pattern* Parser::parse_match_mapping(Node* parent, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchMapping>();
    start_code_loc(pat, token());
    next_token();

    while (token().type() != '}') {

        if (token().operator_name() == "**") {
            next_token();
            pat->rest = token().identifier();
            expect_token(tok_identifier, true, pat, LOC);
            break;
        }

        auto key = parse_expression(pat, depth + 1);

        expect_token(':', true, pat, LOC);
        auto child = parse_pattern(pat, depth + 1);

        pat->keys.push_back(key);
        pat->patterns.push_back(child);

        if (token().type() == tok_comma) {
            next_token();
        } else {
            break;
        }
    }

    end_code_loc(pat, token());
    expect_token('}', true, pat, LOC);
    return pat;
}

// <pattern> | <pattern> | ...
Pattern* Parser::parse_match_or(Node* parent, Pattern* child, int depth) {
    TRACE_START();

    auto pat = parent->new_object<MatchOr>();
    pat->patterns.push_back(child);

    // TODO: this is the loc of '|' not the start of the expression
    start_code_loc(pat, token());

    if (token().operator_name() != "|") {
        kwerror((this->parsinglog), "Unexpected operator {}", token().operator_name());
    }

    next_token();

    while (token().type() != ':') {
        child = parse_pattern(pat, depth + 1);
        pat->patterns.push_back(child);

        if (token().type() == tok_operator && token().operator_name() == "|") {
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

Pattern* Parser::parse_match_as(Node* parent, Pattern* primary, int depth) {
    auto pat     = parent->new_object<MatchAs>();   
    pat->pattern = primary;

    start_code_loc(pat, token());
    expect_token(tok_as, true, pat, LOC);;
    pat->name = get_identifier();

    end_code_loc(pat, token());
    expect_token(tok_identifier, true, pat, LOC);
    return pat;
}

// <Pattern> as <identifier>
Pattern* Parser::parse_match_as(Node* parent, int depth){
    // case a as b => MatchAs(pattern=MatchAs(a), name=c)
    auto pat     = parent->new_object<MatchAs>();

    // TODO: this is the loc of 'as' not the start of the expression
    start_code_loc(pat, token());
    pat->name = get_identifier();
    next_token();

    // case a as b
    if (token().type() == tok_as) {
        end_code_loc(pat, token());
        expect_token(tok_as, true, pat, LOC);

        auto toppat = parent->new_object<MatchAs>();
        start_code_loc(toppat, token());

        toppat->pattern = pat;
        toppat->name = get_identifier();

        end_code_loc(toppat, token());
        expect_token(tok_identifier, true, toppat, LOC);
        return toppat;
    }

    return pat;
}

Pattern* Parser::parse_pattern_1(Node* parent, int depth) {

    switch (token().type()) {
    case tok_square: return parse_match_sequence(parent, depth);

    // TODO: make sure those are correct
    case tok_curly: return parse_match_mapping(parent, depth);

    // rest operator
    // case '_': 

    case tok_operator:
    case tok_star:
        if (token().operator_name() == "**" || token().operator_name() == "*") {
            return parse_match_star(parent, depth);
        }

    // Constants
    // The values represented can be simple types such as a number, string or None,
    // but also immutable container types (tuples and frozensets)
    // if all of their elements are constant.
    case tok_int:
    case tok_string:
    case tok_float: {
        auto pat   = parent->new_object<MatchSingleton>();
        pat->value= get_value(pat);
        next_token();
        return pat;
    }

    case tok_identifier: {
        // case name()
        if (peek_token().type() == tok_parens) {
            return parse_match_class(parent, depth);
        }

        // case name ...
        return parse_match_as(parent, depth);
    }
    // MatchClass is expecting a expression not an identifier
    // this is interesting does that mean if I call a function returning a type
    // it will match on that type ?

    // Value
    // default: {
    //     // in this context we are storing components inside the pattern
    //     //_context.push_back(ExprContext::Store);
    //     auto value = parse_expression_primary(parent, depth + 1);
    //     //_context.pop_back();
    //     Pattern* pat = nullptr;

    //     // <expr> if|:
    //     if (token().type() != '(') {
    //         pat                       = parent->new_object<MatchValue>();
    //         ((MatchValue*)pat)->value = value;
    //         set_context(value, ExprContext::Store);
    //     } else {
    //         pat = parse_match_class(parent, value, depth + 1);
    //     }
    //     value->move(pat);
    //     return pat;
    // }
    }
    kwerror((this->parsinglog), "unknown pattern for");
    return nullptr;
}

Pattern* Parser::parse_pattern(Node* parent, int depth) {
    TRACE_START();

    auto primary = parse_pattern_1(parent, depth);

    switch (token().type()) {

    case tok_as: 
        return parse_match_as(parent, primary, depth);

    case tok_operator:
    case '|':
        if (token().operator_name() == "|") {
            return parse_match_or(parent, primary, depth);
        }
    }
    // could be ":" or "if"
    // expect_token(':', false, primary, LOC);
    return primary;
}

Token Parser::parse_match_case(Node* parent, Array<MatchCase>& out, int depth) {
    TRACE_START();

    Token last = token();

    while (token().type() != tok_desindent) {
        MatchCase case_;
        expect_token(tok_case, true, parent, LOC);
        // Patern
        case_.pattern = parse_pattern(parent, depth);

        // Guard
        if (token().type() == tok_if) {
            next_token();
            case_.guard = parse_expression(parent, depth + 1);
        }

        expect_token(':', true, parent, LOC);
        MAYBE_COMMENT(&case_, comment);
        expect_newline(parent, LOC);
        expect_token(tok_indent, true, parent, LOC);

        // Branch
        last = parse_body(parent, case_.body, depth + 1);
        out.push_back(case_);

        // Stop if next token is not for another branch
        if (token().type() != tok_case) {
            expect_tokens({tok_desindent, tok_eof}, false, parent, LOC);
            break;
        }
    }

    return last;
}

StmtNode* Parser::parse_match(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Match>();
    start_code_loc(stmt, token());
    next_token();

    stmt->subject = parse_expression(stmt, depth + 1);
    expect_token(':', true, stmt, LOC);
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, parent, LOC);

    auto last = parse_match_case(stmt, stmt->cases, depth);

    expect_tokens({tok_desindent, tok_eof}, true, stmt, LOC);
    end_code_loc(stmt, last);
    TRACE_END();
    return stmt;
}

void Parser::parse_withitem(Node* parent, Array<WithItem>& out, int depth) {
    TRACE_START();

    while (token().type() != ':') {
        ExprNode* expr = parse_expression(parent, depth + 1);
        ExprNode* var  = nullptr;

        if (token().type() == tok_as) {
            next_token();
            var = parse_expression_primary(parent, depth + 1);
        }

        out.push_back(WithItem{expr, var});

        if (token().type() == tok_comma) {
            next_token();
        } else {
            break;
        }
    }
}

StmtNode* Parser::parse_with(Node* parent, int depth) {
    TRACE_START();

    With* stmt = nullptr;
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
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);
    end_code_loc(stmt, token());

    return stmt;
}

StmtNode* Parser::parse_raise(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Raise>();
    start_code_loc(stmt, token());
    next_token();

    if (!is_tok_statement_ender()) {
        stmt->exc = parse_expression(stmt, depth + 1);

        if (token().type() == tok_from) {
            next_token();
            stmt->cause = parse_expression(stmt, depth + 1);
        }

        end_code_loc(stmt, token());
    } else {
        end_code_loc(stmt, token());

        if (token().type() != tok_comment) {
            next_token();
        }
    }

    return stmt;
}

Token Parser::parse_except_handler(Try* parent, Array<ExceptHandler>& out, int depth) {
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
        add_inline_comment(parent, &handler, depth);
        expect_newline(parent, LOC);
        expect_token(tok_indent, true, parent, LOC);

        parse_body(parent, handler.body, depth + 1);

        out.push_back(handler);
    };

    return token();
}

StmtNode* Parser::parse_try(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Try>();
    start_code_loc(stmt, token());
    next_token();

    expect_token(':', true, stmt, LOC);
    expect_comment_or_newline(stmt, depth, LOC);
    expect_token(tok_indent, true, stmt, LOC);

    auto last = parse_body(stmt, stmt->body, depth + 1);

    expect_token(tok_except, false, stmt, LOC);
    parse_except_handler(stmt, stmt->handlers, depth + 1);

    if (token().type() == tok_else) {
        next_token();  // else
        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, else_comment);
        expect_newline(stmt, LOC);

        expect_token(tok_indent, true, stmt, LOC);
        parse_body(stmt, stmt->orelse, depth + 1);
    }

    if (token().type() == tok_finally) {
        next_token();  // finally
        expect_token(':', true, stmt, LOC);

        MAYBE_COMMENT(stmt, finally_comment);
        expect_newline(stmt, LOC);

        expect_token(tok_indent, true, stmt, LOC);
        parse_body(stmt, stmt->finalbody, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_assert(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Assert>();
    start_code_loc(stmt, token());
    next_token();

    //
    stmt->test = parse_expression(stmt, depth + 1);
    //

    if (token().type() == tok_comma) {
        next_token();
        stmt->msg = parse_expression(stmt, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
}

bool is_dot(Token const& tok) {
    return (tok.type() == tok_operator && tok.operator_name() == ".") || tok.type() == tok_dot;
}

String Parser::parse_module_path(Node* parent, int& level, int depth) {
    level = 0;
    Array<String> path;
    bool          last_was_dot = false;

    // import <path> as adasd
    // from <path> import cd as xyz

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

            // need an identifier after a `.`
            if (token().type() != tok_identifier) {
                ParsingError& error = parser_kwerror(  //
                    LOC,                               //
                    "SyntaxError",                     //
                    "expect name after ."              //
                );
                add_wip_expr(error, parent);
                PARSER_THROW(SyntaxError, error);
            }
        }

        // we have reached the en dof the path
        if (in(token().type(), tok_import, tok_as, tok_newline, tok_eof)) {
            break;
        }

        //
        expect_tokens({tok_dot, tok_identifier}, false, parent, LOC);
    }

    return join(".", path);
}

void Parser::parse_alias(Node* parent, Array<Alias>& out, int depth) {
    TRACE_START();

    while (!is_tok_statement_ender()) {
        Alias alias;

        int level  = 0;
        alias.name = parse_module_path(parent, level, depth);

        if (token().type() == tok_as) {
            next_token();
            alias.asname = get_identifier();
            expect_token(tok_identifier, true, parent, LOC);
        }

        out.push_back(alias);

        if (token().type() == ',') {
            next_token();
            if (token().type() != tok_identifier) {
                ParsingError& error = parser_kwerror(  //
                    LOC,                               //
                    "SyntaxError",                     //
                    "Expect identifier after ,"        //
                );
                add_wip_expr(error, parent);
                PARSER_THROW(SyntaxError, error);
            }
        } else {
            break;
        }
    }

    if (out.size() <= 0) {
        ParsingError& error = parser_kwerror(  //
            LOC,                               //
            "SyntaxError",                     //
            "Expect packages"                  //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    }
}

StmtNode* Parser::parse_import(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Import>();
    start_code_loc(stmt, token());
    next_token();

    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    add_inline_comment(stmt, depth);
    expect_tokens({tok_newline, tok_eof}, true, stmt, LOC);
    return stmt;
}

StmtNode* Parser::parse_import_from(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<ImportFrom>();
    start_code_loc(stmt, token());
    next_token();

    int level    = 0;
    stmt->module = parse_module_path(parent, level, depth);
    if (level > 0) {
        stmt->level = level;
    }

    expect_token(tok_import, true, stmt, LOC);
    parse_alias(stmt, stmt->names, depth + 1);

    end_code_loc(stmt, token());
    add_inline_comment(stmt, depth);
    expect_tokens({tok_newline, tok_eof}, true, stmt, LOC);
    return stmt;
}

StmtNode* Parser::parse_global(Node* parent, int depth) {
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

StmtNode* Parser::parse_nonlocal(Node* parent, int depth) {
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

StmtNode* Parser::parse_return(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Return>();
    start_code_loc(stmt, token());
    next_token();

    if (!is_tok_statement_ender()) {
        stmt->value = parse_expression(stmt, depth + 1, true);
        end_code_loc(stmt, token());
    } else {
        end_code_loc(stmt, token());

        if (token().type() != tok_comment) {
            next_token();
        }
    }

    TRACE_END();
    return stmt;
}

StmtNode* Parser::parse_del(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Delete>();
    start_code_loc(stmt, token());

    while (!is_tok_statement_ender()) {
        next_token();

        auto expr = parse_expression(stmt, depth + 1);
        stmt->targets.push_back(expr);

        if (token().type() == ',') {
            continue;
        }
    }

    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_pass(Node* parent, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Pass>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_break(Node* parent, int depth) {
    TRACE_START();

#if AVOID_DUPLICATE_CONST
    static Break b;
    return &b;
#endif

    auto stmt = parent->new_object<Break>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

StmtNode* Parser::parse_continue(Node* parent, int depth) {
    TRACE_START();

#if AVOID_DUPLICATE_CONST
    static Continue c;
    return &c;
#endif

    auto stmt = parent->new_object<Continue>();
    start_code_loc(stmt, token());
    end_code_loc(stmt, token());
    next_token();
    return stmt;
}

// Statement_2
StmtNode* Parser::parse_assign(Node* parent, ExprNode* expr, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<Assign>();
    set_context(expr, ExprContext::Store);
    stmt->targets.push_back(expr);

    // FIXME: this is the location of '=' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->value = parse_expression(stmt, depth + 1, true);
    end_code_loc(stmt, token());
    return stmt;
}

StmtNode* Parser::parse_augassign(Node* parent, ExprNode* expr, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<AugAssign>();

    // this is a load-store
    // set_context(expr, ExprContext::Load);
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

StmtNode* Parser::parse_annassign(Node* parent, ExprNode* expr, int depth) {
    TRACE_START();

    auto stmt = parent->new_object<AnnAssign>();
    set_context(expr, ExprContext::Store);
    stmt->target = expr;

    // FIXME: this is the location of :' not the start of the full expression
    start_code_loc(stmt, token());
    next_token();

    stmt->annotation = parse_expression(stmt, depth + 1);

    if (token().type() == tok_assign) {
        expect_token(tok_assign, true, stmt, LOC);
        stmt->value = parse_expression(stmt, depth + 1);
    }

    end_code_loc(stmt, token());
    return stmt;
}

// parse_expression_1
ExprNode* Parser::parse_name(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Name>();
    start_code_loc(expr, token());

    expr->id  = get_identifier();
    expr->ctx = context();

    end_code_loc(expr, token());
    expect_token(tok_identifier, true, expr, LOC);
    return expr;
}

#define LY_UINT64_MAX sizeof("18446744073709551615") / sizeof(char)
#define LY_UINT32_MAX sizeof("4294967295") / sizeof(char)
#define LY_UINT16_MAX sizeof("65535") / sizeof(char)
#define LY_UINT8_MAX  sizeof("255") / sizeof(char)

// +1 char for the minus
#define LY_INT64_MAX sizeof("9223372036854775807") / sizeof(char)
#define LY_INT32_MAX sizeof("2147483647") / sizeof(char)
#define LY_INT16_MAX sizeof("32767") / sizeof(char)
#define LY_INT8_MAX  sizeof("255") / sizeof(char)

bool Parser::is_valid_value() {
    String const& value    = token().identifier();
    int           has_sign = value[0] == '-' || value[0] == '+';

    switch (token().type()) {
    case tok_string: {
        return true;
    }
    case tok_int: {
        if (value.size() > 18 + has_sign) {
            return false;
        }
        return true;
    }
    case tok_float: {
        // Max numbers of digits
        if (value.size() > 16 + has_sign) {
            return false;
        }
        return true;
    }
    case tok_none:
    case tok_true:
    case tok_false: return true;
    }

    return false;
}

Value Parser::get_value(Node* parent) {
    if (!is_valid_value()) {
        ParsingError& error = parser_kwerror(  //
            LOC,                               //
            "SyntaxError",                     //
            "Value is out of range"            //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    }

    switch (token().type()) {

    case tok_string: {
        return make_value<String>(token().identifier());
    }
    case tok_int: {
        // FIXME handle different sizes
        // SEMA should probably accept different size as well
        return make_value<int32>(int32(token().as_integer()));
    }
    case tok_float: {
        return make_value<float64>(token().as_float());
    }
    case tok_none: return make_value<_None>();

    case tok_true: return make_value<bool>(true);

    case tok_false: return make_value<bool>(false);
    }

    return make_value<_None>();
}

ExprNode* Parser::parse_constant(Node* parent, int depth) {
    TRACE_START();

#if AVOID_DUPLICATE_CONST
    // Shortcut
    static Constant none   = Constant(ConstantValue::none_t());
    static Constant truev  = Constant(true);
    static Constant falsev = Constant(false);

    switch (token().type()) {
    case tok_none: return none;

    case tok_true: return truev;

    case tok_false: return falsev;
    }
//
#endif
    auto expr = parent->new_object<Constant>();
    start_code_loc(expr, token());

    expr->value = get_value(expr);

    end_code_loc(expr, token());
    next_token();
    return expr;
}

ExprNode* Parser::parse_await(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Await>();
    start_code_loc(expr, token());
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_yield(Node* parent, int depth) {
    TRACE_START();

    if (peek_token().type() == tok_from) {
        return parse_yield_from(parent, depth);
    }

    auto expr = parent->new_object<Yield>();
    start_code_loc(expr, token());
    next_token();

    if (!is_tok_statement_ender()) {
        expr->value = parse_expression(expr, depth + 1, true);
        end_code_loc(expr, token());
    } else {
        end_code_loc(expr, token());

        if (token().type() != tok_comment) {
            next_token();
        }
    }

    return expr;
}

ExprNode* Parser::parse_yield_from(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<YieldFrom>();
    start_code_loc(expr, token());
    next_token();  // eat yield

    expect_token(tok_from, true, expr, LOC);
    expr->value = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

bool is_star(Token const& tok) { return tok.type() == tok_operator && tok.operator_name() == "*"; }

bool is_starstar(Token const& tok) {
    return tok.type() == tok_operator && tok.operator_name() == "**";
}

Arguments Parser::parse_arguments(Node* parent, char kind, int depth) {
    TRACE_START();

    Arguments args;
    // ArgumentKind kind = ArgumentKind::Regular;
    //

    bool keywords = false;

    while (token().type() != kind) {
        ExprNode* value = nullptr;

        Arg arg;

        bool vararg = false;
        bool kwarg  = false;

        if (token().type() == tok_comma) {
            next_token();
        }

        if (token().type() == tok_operator && token().identifier() == "/") {
            args.posonlyargs = args.args;
            args.args.clear();
            next_token();
            continue;
        }

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
            value = parse_expression(parent, depth + 1);
        }

        if (vararg) {
            args.vararg = arg;
            // only kwargs from that point onward
            keywords = true;
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
            // NB: value can be null here
            args.kw_defaults.push_back(value);
        }
    }

    //
    expect_token(kind, true, parent, LOC);
    return args;
}

ExprNode* Parser::parse_lambda(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Lambda>();
    start_code_loc(expr, token());
    next_token();

    expr->args = parse_arguments(expr, ':', depth + 1);
    expr->body = parse_expression(expr, depth + 1);
    end_code_loc(expr, token());
    return expr;
}

ExprNode* Parser::parse_starred(Node* parent, int depth) {
    TRACE_START();

    auto expr = parent->new_object<Starred>();
    next_token();

    expr->value = parse_expression(expr, depth + 1);
    return expr;
}

void Parser::parse_comprehension(Node* parent, Array<Comprehension>& out, char kind, int depth) {
    TRACE_START();

    PopGuard _(parsing_context, ParsingContext::Comprehension);

    while (token().type() != kind) {
        expect_token(tok_for, true, parent, LOC);
        Comprehension cmp;

        cmp.target = parse_star_targets(parent, depth + 1);
        set_context(cmp.target, ExprContext::Store);

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
ExprNode* parse_comprehension(Parser* parser, Node* parent, ExprNode* child, char kind, int depth) {
    auto expr = parent->new_object<Comp>();

    expr->elt = child;
    parser->parse_comprehension(expr, expr->generators, kind, depth);
    parser->end_code_loc(expr, parser->token());

    parser->expect_token(kind, true, parent, LOC);
    return expr;
}

ExprNode* parse_dictcomprehension(
    Parser* parser, Node* parent, ExprNode* key, ExprNode* value, char kind, int depth) {
    auto expr = parent->new_object<DictComp>();

    expr->key   = key;
    expr->value = value;
    parser->parse_comprehension(expr, expr->generators, kind, depth);
    parser->end_code_loc(expr, parser->token());

    parser->expect_token(kind, true, parent, LOC);
    return expr;
}

template <typename Literal>
ExprNode* parse_literal(Parser* parser, Node* parent, ExprNode* child, char kind, int depth) {
    // TRACE_START2(parser->token());

    // This is a tuple
    auto expr = parent->new_object<Literal>();

    if (child) {
        expr->elts.push_back(child);
    }
    // {a, b, c}
    // [a, b, c]
    // (a, b, c)
    while (true) {
        expr->elts.push_back(parser->parse_expression(parent, depth + 1));

        if (parser->token().type() == ',') {
            parser->next_token();
        } else {
            if (kind) {
                parser->expect_token(kind, true, expr, LOC);
            }
            break;
        }
    }

    parser->end_code_loc(expr, parser->token());
    // TRACE_END2(parser->token());
    return expr;
}

ExprNode* parse_dictliteral(
    Parser* parser, Node* parent, ExprNode* key, ExprNode* value, char kind, int depth) {
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
ExprNode*
parse_comprehension_or_literal(Parser* parser, Node* parent, int tok, char kind, int depth) {
    // Save the start token to set the code loc when we know if this is a tuple or a generator
    auto start_tok = parser->token();
    SHOW_TOK(parser, start_tok);
    parser->expect_token(tok, true, nullptr, LOC);  // eat (  [  {

    // Warning: the parent is wrong but we need to parse the expression right now
    auto      child      = parser->parse_expression(parent, depth + 1);
    ExprNode* value      = nullptr;
    bool      dictionary = false;

    // Dictionary
    if (parser->token().type() == ':') {
        parser->next_token();
        value      = parser->parse_expression(parent, depth + 1);
        dictionary = true;
    }
    // ----

    //
    ExprNode* expr = nullptr;
    if (parser->token().type() == tok_for) {
        // This is generator comprehension
        if (dictionary) {
            expr = parse_dictcomprehension(parser, parent, child, value, kind, depth);
        } else {
            expr = parse_comprehension<Comp>(parser, parent, child, kind, depth);
            kwerror((parser->parsinglog), "Done {}", str(expr));
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
            // fixme this miss the call
            auto p = parser->parse_expression_1(parent, child, 0, depth);
            parser->expect_token(')', true, parent, LOC);

            if (p)
                return p;

            return child;
        }

        kwerror((parser->parsinglog), "Unhandled list-comprehension case");
    }

    if (expr == nullptr) {
        ParsingError& error = parser->parser_kwerror(  //
            LOC,                                       //
            "SyntaxError",                             //
            "Comprehension is null"                    //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    }

    // fix the things we could not do at the begining
    parser->end_code_loc(expr, parser->token());
    parser->start_code_loc(expr, start_tok);
    child->move(expr);
    // ----------------------------------------------

    SHOW_TOK(parser, parser->token());
    return expr;
}

// [a, b] or [a for b in c]
ExprNode* Parser::parse_list(Node* parent, int depth) {
    TRACE_START();
    return parse_comprehension_or_literal<ListComp, ListExpr>(
        this, parent, tok_square, ']', depth + 1);
}

// (a, b) or (a for b in c) or (a + b)
ExprNode* Parser::parse_tuple_generator(Node* parent, int depth) {
    TRACE_START();

    // auto expr = parse_expression_primary(parent, depth + 1);

    return parse_comprehension_or_literal<GeneratorExp, TupleExpr>(
        this, parent, tok_parens, ')', depth + 1);
}

// {a, b} or {a for b in c} or {a: b, c: d} or {a: b for a, b in c}
ExprNode* Parser::parse_set_dict(Node* parent, int depth) {
    TRACE_START();
    return parse_comprehension_or_literal<SetComp, SetExpr>(this, parent, tok_curly, '}', depth);
}

ExprNode* Parser::parse_ifexp(Node* parent, ExprNode* primary, int depth) {

    // if is part of the comprehension
    if (parsing_context.size() > 0 &&
        parsing_context[parsing_context.size() - 1] == ParsingContext::Comprehension) {
        return nullptr;
    }

    // body if test else body
    IfExp* expr = parent->new_object<IfExp>();
    start_code_loc(expr, token());
    next_token();

    primary->move(expr);
    expr->body = primary;

    expr->test = parse_expression(expr, depth + 1);

    expect_token(tok_else, true, expr, LOC);
    expr->orelse = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

// parse_expression_2
ExprNode* Parser::parse_named_expr(Node* parent, ExprNode* primary, int depth) {
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

ExprNode* Parser::parse_bool_operator(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode* Parser::parse_binary_operator(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode* Parser::parse_compare_operator(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}
ExprNode* Parser::parse_suffix_unary(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();
    return not_implemented_expr(parent);
}

ExprNode* Parser::parse_prefix_unary(Node* parent, int depth) {
    TRACE_START();
    auto expr = parent->new_object<UnaryOp>();
    start_code_loc(expr, token());

    auto conf = get_operator_config(token());

    if (token().operator_name() == "*") {
        return parse_starred(parent, depth);
    }

    if (conf.unarykind == UnaryOperator::None) {
        ParsingError& error = parser_kwerror(            //
            LOC,                                         //
            "SyntaxError",                               //
            fmtstr("Expected an unary operator not {}",  //
                   str(token())));

        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    }

    next_token();

    expr->op      = conf.unarykind;
    expr->operand = parse_expression(expr, depth + 1);

    end_code_loc(expr, token());
    return expr;
}

Token Parser::parse_call_args(Node*             expr,
                              Array<ExprNode*>& args,
                              Array<Keyword>&   keywords,
                              int               depth) {
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
            kwarg.arg  = get_identifier();  // <= NB: this checks for tok_identifier
                                            // if not returns a dummy identifier
            expect_token(tok_identifier, true, expr, LOC);
            expect_token(tok_assign, true, expr, LOC);

            kwarg.value = parse_expression(expr, depth + 1);
            keywords.push_back(kwarg);
        }

        if (token().type() == ',') {
            next_token();
        } else {
            break;
        }
    }

    auto last = token();
    expect_token(')', true, expr, LOC);
    return last;
}

ExprNode* Parser::parse_call(Node* parent, ExprNode* primary, int depth) {
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

ExprNode* Parser::parse_attribute(Node* parent, ExprNode* primary, int depth) {
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

ExprNode* Parser::parse_subscript(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();

    auto expr   = parent->new_object<Subscript>();
    expr->value = primary;

    // FIXME: this is the location of '[' not the start of the full expression
    start_code_loc(expr, token());
    expect_token(tok_square, true, expr, LOC);

    // We do not allocate the TupleExpr unless required
    Array<ExprNode*> elts;

    // a[2:3]       => Slice(2, 3)
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1:2, 2:3]  => Tuple(Slice(1, 2), Slice(2, 3))
    // a[1, 2, 3]   => Tuple(1, 2, 3)
    //
    {
        PopGuard _(parsing_context, ParsingContext::Slice);

        while (token().type() != ']') {
            elts.push_back(parse_expression(expr, depth + 1));

            if (token().type() == ',') {
                next_token();
            } else {
                expect_token(']', true, expr, LOC);
                break;
            }
        }
    }

    if (elts.size() == 0) {
        ParsingError& error = parser_kwerror(        //
            LOC,                                     //
            "SyntaxError",                           //
            "Substript needs at least one argument"  //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
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

void Parser::error_recovery(ParsingError* error) {
    while (!in(token().type(), tok_newline, tok_eof)) {
        error->remaining.push_back(token());
        next_token();
    }
    error->line = currentline.tokens;

    if (error->line.size() > 0) {
        Token const& start = error->line[0];
        Token const& end   = error->line[int(error->line.size() - 1)];

        // then we got a new line
        if (!error->received_token.isbetween(start, end)) {
            error->received_token = end;
        }
    } else {
        kwerror((this->parsinglog), "Was not able to retrieve the tok line for an error");
    }
}

ExprNode* Parser::parse_slice(Node* parent, ExprNode* primary, int depth) {
    TRACE_START();

    if (!allow_slice()) {
        ParsingError& error = parser_kwerror(       //
            LOC,                                    //
            "SyntaxError",                          //
            "Slice is not allowed in this context"  //
        );
        add_wip_expr(error, primary);
        PARSER_THROW(SyntaxError, error);

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

void set_decorators(StmtNode* stmt, Array<Decorator>& decorators) {
    if (decorators.size() > 0) {
        if (stmt->kind == NodeKind::FunctionDef) {
            auto fun            = cast<FunctionDef>(stmt);
            fun->decorator_list = decorators;

        } else if (stmt->kind == NodeKind::ClassDef) {
            auto cls            = cast<ClassDef>(stmt);
            cls->decorator_list = decorators;
        }
    }
}

StmtNode* Parser::parse_statement(Node* parent, int depth) {

    TRACE_START();

    Array<Decorator> decorators;
    while (token().type() == tok_decorator) {
        next_token();

        Comment* comment = nullptr;
        auto     fun     = parse_expression(parent, depth);

        if (token().type() == tok_comment) {
            comment = parse_comment(fun, depth);
        }

        decorators.emplace_back(fun, comment);

        if (token().type() == tok_newline) {
            next_token();
        }
    }

    auto stmt = parse_statement_primary(parent, depth + 1);
    set_decorators(stmt, decorators);

    if (token().type() != ';') {
        TRACE_END();
        return stmt;
    }

    Array<StmtNode*> body;
    body.push_back(stmt);

    while (token().type() == ';') {
        next_token();

        if (is_tok_statement_ender()) {
            break;
        }

        stmt = parse_statement_primary(parent, depth + 1);
        body.push_back(stmt);
    }

    stmt = nullptr;
    if (body.size() == 1) {
        stmt = body[0];
    } else {
        auto inlinestmt  = parent->new_object<Inline>();
        inlinestmt->body = body;
        stmt             = inlinestmt;
    }

    TRACE_END();
    return stmt;
}

StmtNode* Parser::parse_yield_stmt(Node* parent, int depth) {
    // yield is an expression in the AST but it can be parsed
    // as simple_statement or an expression
    auto stmt   = parent->new_object<Expr>();
    stmt->value = parse_yield(stmt, depth);
    return stmt;
}

StmtNode* Parser::parse_comment_stmt(Node* parent, int depth) {
    Expr*     comment_stmt = parent->new_object<Expr>();
    ExprNode* comment      = parse_comment(comment_stmt, depth);

    comment_stmt->value = comment;

    expect_tokens({tok_newline, tok_eof}, true, comment_stmt, LOC);
    return comment_stmt;
}

StmtNode* Parser::parse_statement_primary(Node* parent, int depth) {
    TRACE_START();

    if (previous == token()) {
        ParsingError& error = parser_kwerror(                        //
            LOC,                                                     //
            "SyntaxError",                                           //
            fmtstr("Unhandled token {} `{}` previous tok was `{}`",  //
                   token().type(),                                   //
                   str(token()),                                     //
                   str(previous)                                     //
                   )                                                 //
        );
        add_wip_expr(error, parent);
        PARSER_THROW(SyntaxError, error);
    } else {
        previous = token();
    }

    // Statement we can guess rightaway from the current token we are seeing
    switch (token().type()) {
        // clang-format off

    // Small Statement
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // assignment:
    //     | NAME ':' expression ['=' annotated_rhs ]
    //     | single_target ':' expression ['=' annotated_rhs ]
    //     | (star_targets '=' )+ (yield_expr | star_expressions) !'=' [TYPE_COMMENT]
    //     | single_target augassign ~ (yield_expr | star_expressions)
    //
    // single_target:
    //     | single_subscript_attribute_target
    //     | NAME
    //     | '(' single_target ')'
    //
    // single_subscript_attribute_target:
    //     | t_primary '.' NAME !t_lookahead
    //     | t_primary '[' slices ']' !t_lookahead
    //
    // star_targets:
    //     | star_target !','
    //     | star_target (',' star_target )* [',']

    // star_target:
    //     | '*' (!'*' star_target)
    //     | target_with_star_atom
    // target_with_star_atom:
    //     | t_primary '.' NAME !t_lookahead
    //     | t_primary '[' slices ']' !t_lookahead
    //     | star_atom
    // star_atom:
    //     | NAME
    //     | '(' target_with_star_atom ')'
    //     | '(' [star_targets_tuple_seq] ')'
    //     | '[' [star_targets_list_seq] ']'
    // star_targets_list_seq: ','.star_target+ [',']
    // star_targets_tuple_seq:
    //     | star_target (',' star_target )+ [',']
    //     | star_target ','

    // star_expressions
    //

    // clang-format on
    case tok_comment: return parse_comment_stmt(parent, depth + 1);
    case tok_return: return parse_return(parent, depth + 1);
    case tok_import: return parse_import(parent, depth + 1);
    case tok_from: return parse_import_from(parent, depth + 1);
    case tok_raise: return parse_raise(parent, depth + 1);
    case tok_pass: return parse_pass(parent, depth + 1);
    case tok_del: return parse_del(parent, depth + 1);
    case tok_yield: return parse_yield_stmt(parent, depth + 1);
    case tok_assert: return parse_assert(parent, depth + 1);
    case tok_break: return parse_break(parent, depth + 1);
    case tok_continue: return parse_continue(parent, depth + 1);
    case tok_global: return parse_global(parent, depth + 1);
    case tok_nonlocal: return parse_nonlocal(parent, depth + 1);
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    // Compound Statement
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // def <name>(...
    case tok_def: return parse_function_def(parent, false, depth + 1);
    // async def <name>(...
    case tok_async: return parse_function_def(parent, true, depth + 1);
    case tok_if: return parse_if_alt(parent, depth + 1);
    case tok_class: return parse_class_def(parent, depth + 1);
    case tok_with: return parse_with(parent, depth + 1);

    // Async for: only valid inside async function
    case tok_for: return parse_for(parent, depth + 1);
    case tok_try: return parse_try(parent, depth + 1);
    case tok_while: return parse_while(parent, depth + 1);
    case tok_match: return parse_match(parent, depth + 1);
    }

    auto expr = parse_expression(parent, depth + 1);
    TRACE_START();

    // allow unpacking
    // promote to a tuple expression
    if (token().type() == tok_comma) {
        next_token();
        expr = parse_literal<TupleExpr>(this, parent, expr, 0, depth + 1);
    }

    switch (token().type()) {
    // <expr> = <>
    case tok_assign: return parse_assign(parent, expr, depth + 1);
    // <expr> += <>
    case tok_augassign: return parse_augassign(parent, expr, depth + 1);
    // <expr>: type = <>
    case ':':
    case tok_annassign: return parse_annassign(parent, expr, depth + 1);
    }

    // fallback to standard expression
    auto stmt_expr   = parent->new_object<Expr>();
    stmt_expr->value = expr;
    TRACE_END();
    return stmt_expr;
}

// Precedence climbing method
// I liked shunting yard algorithm better has it was not recursive
// but it got issues with some edge cases.
//
// https://en.wikipedia.org/wiki/Operator-precedence_parser#:~:text=The%20precedence%20climbing%20method%20is%20a%20compact%2C%20efficient%2C,in%20EBNF%20format%20will%20usually%20look%20like%20this%3A
ExprNode* Parser::parse_operators(Node* og_parent, ExprNode* lhs, int min_precedence, int depth) {
    TRACE_START();

    Node*    parent = og_parent;
    BinOp*   binop  = nullptr;
    Compare* comp   = nullptr;
    BoolOp*  boolop = nullptr;

    // FIXME: For error reporting we need to catch th error here and build the partia expression
    while (true) {
        Token           lookahead = token();
        OpConfig const& op_conf   = get_operator_config(lookahead);
        int             oppred    = op_conf.precedence;

        if (op_conf.type == tok_eof) {
            return lhs;
        }

        // lookahead is a binary operator whose precedence is >= min_precedence
        if (!(is_binary_operator_family(op_conf) && oppred >= min_precedence)) {
            break;
        }

        // we are going to build the operator for sure
        // create the operator right away so we can use save what we have so far for error reporting
        if (op_conf.binarykind != BinaryOperator::None) {
            binop       = parent->new_object<BinOp>();
            binop->left = lhs;
            binop->op   = op_conf.binarykind;

            parent = binop;
        }

        else if (op_conf.cmpkind != CmpOperator::None) {
            // parent is a Comparison (1 < ?expr < ) and we are doing chained comparison
            Compare* lhs_parent = cast<Compare>(parent);
            if (lhs_parent != nullptr) {
                comp = lhs_parent;
                if (comp->safe_comparator_add(lhs)) {
                    comp->ops.push_back(op_conf.cmpkind);
                } else {
                    ParsingError& err = parser_kwerror(        //
                        LOC,                                   //
                        "SyntaxError",                         //
                        fmtstr("Unable to parse comparators")  //
                    );
                    add_wip_expr(err, parent);
                    PARSER_THROW(SyntaxError, err);
                }
            } else {
                comp       = parent->new_object<Compare>();
                comp->left = lhs;
                comp->ops.push_back(op_conf.cmpkind);
            }

            parent = comp;
        }

        else if (op_conf.boolkind != BoolOperator::None) {
            BoolOp* lhs_parent = cast<BoolOp>(parent);

            if (lhs_parent != nullptr && lhs_parent->op == op_conf.boolkind) {
                boolop = lhs_parent;

                if (boolop->safe_value_add(lhs)) {
                    boolop->opcount += 1;
                } else {
                    ParsingError& err = parser_kwerror(                   //
                        LOC,                                              //
                        "SyntaxError",                                    //
                        fmtstr("Unable to finish parsing bool operator")  //
                    );
                    add_wip_expr(err, parent);
                    PARSER_THROW(SyntaxError, err);
                }

            } else {
                boolop          = parent->new_object<BoolOp>();
                boolop->op      = op_conf.boolkind;
                boolop->values  = {lhs};
                boolop->opcount = 1;
            }

            parent = boolop;
        }

        next_token();
        // in the case of 1 < 2 < 3

        auto* rhs = parse_expression(parent, depth);
        lookahead = token();

        auto lookconf    = get_operator_config(lookahead);
        auto lookpred    = lookconf.precedence;
        auto right_assoc = !lookconf.left_associative;

        // lookahead is a binary operator whose precedence is greater
        // than op's, or a right-associative operator
        // whose precedence is equal to op's
        while (lookahead.type() == tok_operator && is_binary_operator_family(lookconf) &&
               (lookpred > oppred || (right_assoc && lookpred == oppred))) {
            rhs = parse_expression_1(parent, rhs, oppred, depth + 1);

            lookahead = token();

            lookconf    = get_operator_config(lookahead);
            lookpred    = lookconf.precedence;
            right_assoc = !lookconf.left_associative;
        }

        // the result of applying op with operands lhs and rhs
        if (op_conf.binarykind != BinaryOperator::None) {
            binop->right = rhs;
            lhs          = binop;
        } else if (op_conf.cmpkind != CmpOperator::None) {
            // rhs can be comp if it was a nested comparison
            // but it does not have to be
            if (rhs != comp) {
                comp->safe_comparator_add(rhs);
            }
            //
            lhs = comp;

        } else if (op_conf.boolkind != BoolOperator::None) {
            if (rhs != boolop) {
                boolop->values.push_back(rhs);
            }
            lhs = boolop;
        } else {
            kwerror((this->parsinglog), "unknow operator {}", str(op_conf));
        }
    }

    TRACE_END();
    return lhs;
}

Comment* Parser::parse_comment(Node* parent, int depth) {
    TRACE_START();

    Comment* com = parent->new_object<Comment>();
    lyassert(token().type() == tok_comment, "Need a comment token");

    com->comment = token().identifier();
    next_token();

    // while (!in(token().type(), tok_newline, tok_eof)) {
    //     com->tokens.push_back(token());
    //     next_token();
    // }

    return com;
}

ExprNode* Parser::parse_expression(Node* parent, int depth, bool comma) {
    TRACE_START();

    expression_depth += 1;
    // parse primary
    auto primary = parse_expression_primary(parent, depth);

    ExprNode* expr = primary;
    while (expr != nullptr) {
        primary = expr;
        expr    = parse_expression_1(parent, primary, 0, depth, comma);
    }

    expression_depth -= 1;
    return primary;
}

// Expression we can guess rightaway from the current token we are seeing
ExprNode* Parser::parse_expression_primary(Node* parent, int depth) {

    switch (token().type()) {
    // await <expr>
    case tok_await: return parse_await(parent, depth);

    // yield from <expr>
    // yield <expr>
    case tok_yield_from:
    case tok_yield: return parse_yield(parent, depth);

    // atom:
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Name: <identifier>
    case tok_identifier: return parse_name(parent, depth);

    case tok_none:
    case tok_true:
    case tok_false:
    case tok_int:
    case tok_float:
    case tok_string: return parse_constant(parent, depth);
    case tok_formatstr: return parse_special_string(parent, depth);

    // List: [a, b]
    // Comprehension [a for a in b]
    case tok_square: return parse_list(parent, depth);

    // Tuple: (a, b)
    // Generator Comprehension: (a for a in b)
    // can be (1 + b)
    case tok_parens: return parse_tuple_generator(parent, depth);

    // Set: {a, b}
    // Comprehension {a for a in b}
    //      OR
    // Dict: {a : b}
    // Comprehension {a: b for a, b in c}
    case tok_curly: return parse_set_dict(parent, depth);

    // TODO: add elipsis

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // lambda <name>:
    case tok_lambda: return parse_lambda(parent, depth);

    // f"
    case tok_fstring: return parse_joined_string(parent, depth);

    // not python syntax
    // if <expr> else <expr>
    case tok_if: {
        if (with_extension)
            return parse_ifexp_ext(parent, depth);
    }

    // *<expr>
    case tok_star:
    case tok_operator: return parse_prefix_unary(parent, depth);
    }

    // Left Unary operator
    // + <expr> | - <expr> | ! <expr> | ~ <expr>

    ParsingError& error = parser_kwerror(  //
        LOC,                               //
        "SyntaxError",                     //
        // fmtstr("Could not deduce the expression {}", str(TokenType(token().type())))  //
        "Expected an expression"  //
    );
    add_wip_expr(error, parent);
    PARSER_THROW(SyntaxError, error);
}

ExprNode* Parser::parse_expression_1(
    Node* parent, ExprNode* primary, int min_precedence, int depth, bool comma) {
    //
    switch (token().type()) {
    case tok_parens: {
        return parse_call(parent, primary, depth);
        break;
    }

    // <expr>.<identifier>
    case tok_dot: {
        return parse_attribute(parent, primary, depth);
    }

    // <expr> := <expr>
    // assign expression instead of the usual assign statement
    case tok_walrus: return parse_named_expr(parent, primary, depth);

    // <expr> boolop <expr>
    /*
    case tok_boolop:
        return parse_bool_operator(parent, primary, depth);
    case tok_binaryop:
        return parse_binary_operator(parent, primary, depth);
    case tok_compareop:
        return parse_compare_operator(parent, primary, depth);
    */
    case tok_unaryop: return parse_suffix_unary(parent, primary, depth);

    case tok_if: return parse_ifexp(parent, primary, depth);
    case tok_in:
    case tok_operator: return parse_operators(parent, primary, min_precedence, depth);

    // <expr>[
    case tok_square: return parse_subscript(parent, primary, depth);

    // this causes more issues than it solves
    // to allow unpacking we will need to move this to somewhere more specific
    case tok_comma: {
        // If we are going deeper we neeed the user to use ( explicitly
        if (comma) {
            next_token();

            auto expr = parse_literal<TupleExpr>(this, parent, primary, '\0', depth);
            return expr;
        }
    }

    // ':' is only valid inside a subscript
    case ':': {
        if (allow_slice())
            return parse_slice(parent, primary, depth);
    }
    }

    return nullptr;
}

ExprNode* Parser::parse_special_string(Node* parent, int depth) {
    TRACE_START();

    String format_type = token().identifier();

    if (format_type == "f") {
        return parse_joined_string(parent, depth);
    }

    if (format_type == "b") {
        next_token();
        return parse_constant(parent, depth);
    }

    if (format_type == "r") {
        next_token();
        return parse_constant(parent, depth);
    }

    return nullptr;
}

struct SetLexerMode {
    SetLexerMode(AbstractLexer& lexer, LexerMode mode): lexer(lexer) {
        old_mode = int(lexer.get_mode());
        lexer.set_mode(int(mode));
    }

    ~SetLexerMode() { lexer.set_mode(old_mode); }

    int            old_mode;
    AbstractLexer& lexer;
};

// FIXME: this can be simplified if the fmtstr lexer tokenize more
ExprNode* Parser::parse_joined_string(Node* parent, int depth) {
    Token tok = token();  // `f`

    JoinedStr* expr = parent->new_object<JoinedStr>();
    start_code_loc(expr, token());
    int8 endquote;

    {
        SetLexerMode _(_lex, LexerMode::Character);
        tok      = next_token();  // Quote
        endquote = token().type();

        tok = next_token();
        String buffer;

        auto pushbuffer = [&]() {
            if (!buffer.empty()) {
                Constant* cst = expr->new_object<Constant>(buffer);
                expr->values.push_back(cst);
                buffer.clear();
            }
        };

        while (tok.type() != endquote) {

            if (tok.type() == '{') {
                char c = _lex.peekc();
                if (c == '{') {
                    buffer.push_back('{');
                    buffer.push_back('{');

                    tok = next_token();
                    tok = next_token();
                    continue;
                }

                pushbuffer();
                // Parse FormattedValue
                expr->values.push_back(parse_formatted_value_string(expr, depth + 1));
                tok = token();
                continue;
            }

            buffer.push_back(tok.type());
            tok = next_token();
        }
        pushbuffer();
    }

    end_code_loc(expr, token());
    expect_token(endquote, true, expr, LOC);
    return expr;
}

ExprNode* Parser::parse_formatted_value_string(Node* parent, int depth) {
    // {<value>(:<formatspec>)?}
    TRACE_START();

    FormattedValue* expr = parent->new_object<FormattedValue>();
    start_code_loc(expr, token());

    {
        SetLexerMode _(_lex, LexerMode::Default);
        expect_token('{', true, expr, LOC);
        expr->value = parse_expression(expr, depth + 1);
    }

    if (token().type() == ':') {
        next_token();  // eat ':' so we get next character
        expr->format_spec = parse_format_spec(expr, depth);
    }

    end_code_loc(expr, token());
    expect_token('}', true, expr, LOC);

    return expr;
}

JoinedStr* Parser::parse_format_spec(Node* parent, int depth) {
    // [[fill]align][sign][#][0][minimumwidth][.precision][type]
    // https://peps.python.org/pep-3101/
    TRACE_START();

    // str{<something>}end
    JoinedStr* expr = parent->new_object<JoinedStr>();
    start_code_loc(expr, token());

    String buffer;

    auto pushbuffer = [&]() {
        if (!buffer.empty()) {
            Constant* cst = expr->new_object<Constant>(buffer);
            expr->values.push_back(cst);
            buffer.clear();
        }
    };

    // Get Current token
    char c     = token().type();
    bool first = true;
    while (c != '}') {

        if (c == '{') {
            pushbuffer();
            {
                SetLexerMode _(_lex, LexerMode::Default);
                if (!first) {
                    next_token();  // token is now {
                }
                expect_token('{', true, expr, LOC);
                expr->values.push_back(parse_expression(expr, depth + 1));
            }

            expect_token('}', true, expr, LOC);
            c     = token().type();
            first = true;
            continue;
        }

        if (!first) {
            c = next_token().type();
        }
        buffer.push_back(c);
        c     = _lex.peekc();
        first = false;
    }

    pushbuffer();

    next_token();  // Current token becomes '}'

    {
        SetLexerMode _(_lex, LexerMode::Default);
        expect_token('}', false, expr, LOC);
    }
    return expr;
}

Token const& Parser::next_token() {
    // add current token to the line and fetch next one
    COZ_BEGIN("T::Lexer::next_token");

    // desindent are issued right after newlines
    is_empty_line = in(token().type(), tok_newline, tok_desindent, tok_indent);

    currentline.add(token());
    Token const& tok = _lex.next_token();

    COZ_PROGRESS_NAMED("Lexer::next_token");
    COZ_END("T::Lexer::next_token");
    return tok;
}

}  // namespace lython
