#ifndef LYTHON_PARSER_H
#define LYTHON_PARSER_H

#include "ast/nodes.h"
#include "dependencies/coz_wrap.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/metadata.h"
#include "utilities/printing.h"

#include <iostream>
#include <numeric>

namespace lython {

enum class ParsingContext
{
    None,
    Comprehension,
    Slice,
};

/* Saves all the token that makes up for the current line
 * this is used for printing better error message
 */
struct TokenBuffer {

    void add(Token const& tok) {
        if (tok.type() == tok_newline) {
            tokens.clear();
        } else {
            tokens.push_back(tok);
        }
    }

    Array<Token> tokens;
};

/**
 * The parser is responsible for transforming the source code into the AST.
 * notifying the users of any syntax error that could prevent it from completing
 * the transformation.
 *
 * SyntaxError will exclusively come from this object.
 *
 * The parser is an handwritten recursive decent parser. It uses the next token
 * to decide which AST-node it should parse next.
 * In case of an error (i.e unexpected token) the parser continues as if was token
 * was defined. This can cause cascading SyntaxError, users should focus on the first one.
 *
 */
class Parser {
    public:
    Parser(AbstractLexer& lexer): _lex(lexer) { metadata_init_names(); }

    Logger& parsinglog = lython::outlog();

    ParsingError&
    parser_kwerror(lython::CodeLocation const& loc, String const& exception, String const& msg) {
        current_error += 1;
        lyassert(current_error == errors.size(), "Only one error at a time can happen");

        auto& details          = errors.emplace_back(ParsingError());
        details.error_kind     = exception;
        details.message        = msg;
        details.loc            = loc;
        details.received_token = token();

        parsinglog.log(lython::LogLevel::Error, loc, "{}: {}", exception, msg);
        return details;
    }

    bool has_errors() const { return errors.size() > 0; }

    void parse_to_module(Module* module) {
        // lookup the module

        try {
            parse_body(module, module->body, 0);
        } catch (ParsingException const&) {
            // this is SyntaxError: Expected a body
            // it was inserted by parse_body
            // and raised to reach the parent block
            // this is the top level block no need to go further up
        }
    }

    // Stream API
    StmtNode* next(Module* module) {
        StmtNode* stmt = parse_one(module, 0);
        module->body.push_back(stmt);
        return stmt;
    }

    Module* parse_module() {
        // lookup the module
        Module* module   = new Module();
        module->class_id = meta::type_id<Module>();

        parse_to_module(module);
        return module;
    }

    Token  parse_body(Node* parent, Array<StmtNode*>& out, int depth);
    Token  parse_except_handler(Try* parent, Array<ExceptHandler>& out, int depth);
    void   parse_alias(Node* parent, Array<Alias>& out, int depth);
    Token  parse_match_case(Node* parent, Array<MatchCase>& out, int depth);
    String parse_module_path(Node* parent, int& level, int depth);

    // Patterns
    Pattern* parse_match_sequence(Node* parent, int depth);
    Pattern* parse_match_star(Node* parent, int depth);
    Pattern* parse_match_mapping(Node* parent, int depth);

    Pattern* parse_match_or(Node* parent, Pattern* primary, int depth);
    Pattern* parse_match_as_or_class(Node* parent, int depth);
    Pattern* parse_match_as(Node* parent, int depth);
    Pattern* parse_match_as(Node* parent, Pattern* primary, int depth);
    Pattern* parse_match_class(Node* parent, int depth);

    // Pattern Dispatch
    Pattern* parse_pattern(Node* parent, int depth);
    Pattern* parse_pattern_1(Node* parent, int depth);

    StmtNode* parse_one(Node* parent, int depth, bool interactive = false);

    // Statement_1
    StmtNode* parse_function_def(Node* parent, bool async, int depth);
    StmtNode* parse_class_def(Node* parent, int depth);
    StmtNode* parse_for(Node* parent, int depth);
    StmtNode* parse_while(Node* parent, int depth);
    StmtNode* parse_if(Node* parent, int depth);
    StmtNode* parse_if_alt(Node* parent, int depth);
    StmtNode* parse_match(Node* parent, int depth);
    StmtNode* parse_with(Node* parent, int depth);
    StmtNode* parse_raise(Node* parent, int depth);
    StmtNode* parse_try(Node* parent, int depth);
    StmtNode* parse_assert(Node* parent, int depth);
    StmtNode* parse_import(Node* parent, int depth);
    StmtNode* parse_import_from(Node* parent, int depth);
    StmtNode* parse_global(Node* parent, int depth);
    StmtNode* parse_nonlocal(Node* parent, int depth);
    StmtNode* parse_return(Node* parent, int depth);
    StmtNode* parse_del(Node* parent, int depth);
    StmtNode* parse_pass(Node* parent, int depth);
    StmtNode* parse_break(Node* parent, int depth);
    StmtNode* parse_continue(Node* parent, int depth);
    StmtNode* parse_comment_stmt(Node* parent, int depth);

    // Statement_2
    StmtNode* parse_assign(Node* parent, ExprNode* epxr, int depth);
    StmtNode* parse_augassign(Node* parent, ExprNode* epxr, int depth);
    StmtNode* parse_annassign(Node* parent, ExprNode* epxr, int depth);

    // Statement Dispatch
    // ------------------
    StmtNode* parse_statement(Node* parent, int depth);
    StmtNode* parse_statement_primary(Node* parent, int depth);

    // Primary expression
    // parse_expression_1
    Comment*  parse_comment(Node* parent, int depth);
    ExprNode* parse_await(Node* parent, int depth);
    StmtNode* parse_yield_stmt(Node* parent, int depth);
    ExprNode* parse_yield(Node* parent, int depth);
    ExprNode* parse_yield_from(Node* parent, int depth);
    ExprNode* parse_name(Node* parent, int depth);
    ExprNode* parse_lambda(Node* parent, int depth);
    ExprNode* parse_constant(Node* parent, int depth);
    ExprNode* parse_ifexp_ext(Node* parent, int depth);

    ExprNode*  parse_special_string(Node* parent, int depth);
    ExprNode*  parse_joined_string(Node* parent, int depth);
    ExprNode*  parse_formatted_value_string(Node* parent, int depth);
    JoinedStr* parse_format_spec(Node* parent, int depth);

    ExprNode* parse_starred(Node* parent, int depth);
    ExprNode* parse_list(Node* parent, int depth);
    ExprNode* parse_tuple_generator(Node* parent, int depth);
    ExprNode* parse_set_dict(Node* parent, int depth);
    ExprNode* parse_prefix_unary(Node* parent, int depth);

    void      parse_comprehension(Node* parent, Array<Comprehension>& out, char kind, int depth);
    Arguments parse_arguments(Node* parent, char kind, int depth);
    Token
         parse_call_args(Node* parent, Array<ExprNode*>& args, Array<Keyword>& keywords, int depth);
    void parse_withitem(Node* parent, Array<WithItem>& out, int depth);
    ExprNode* parse_star_targets(Node* parent, int depth);

    // parse_expression_2
    // TODO: primary has the parent has GC not the expression it belongs to
    ExprNode* parse_named_expr(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_bool_operator(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_binary_operator(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_compare_operator(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_suffix_unary(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_call(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_attribute(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_subscript(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_slice(Node* parent, ExprNode* primary, int depth);
    ExprNode* parse_star_expression(Node* parent, int depth);
    ExprNode* parse_ifexp(Node* parent, ExprNode* primary, int depth);

    // Expression Dispatcher
    // ---------------------
    // using the current token dispatch to the correct parsing routine
    ExprNode* parse_expression(Node* parent, int depth, bool comma = false);

    ExprNode* parse_expression_primary(Node* parent, int depth);

    ExprNode* parse_expression_1(
        Node* parent, ExprNode* primary, int min_precedence, int depth, bool comma = false);

    ExprNode* parse_operators(Node* parent, ExprNode* lhs, int min_precedence, int depth);

    // Helpers
    // -------
    void start_code_loc(CommonAttributes* target, Token tok);

    void end_code_loc(CommonAttributes* target, Token tok);

    bool                       is_valid_value();
    Value get_value(Node* parent);

    OpConfig const& get_operator_config(Token const& tok) const;

    bool is_binary_operator_family(OpConfig const& conf);

    void add_inline_comment(StmtNode* stmt, int depth) {
        //
        add_inline_comment(stmt, stmt, depth);
    }

    template <typename T>
    void add_inline_comment(Node* parent, T* stmt, int depth) {
        if (token().type() == tok_comment) {
            stmt->comment = parse_comment(parent, depth + 1);
        }
    }

    // Error Handling
    // --------------
    void expect_token(int expected, bool eat, Node* wip_expression, CodeLocation const& loc);

    void expect_operator(String const& op, bool eat, Node* wip_expression, CodeLocation const& loc);

    void expect_tokens(Array<int> const&   expected,
                       bool                eat,
                       Node*               wip_expression,
                       CodeLocation const& loc);

    // check for a new lines and eats all the subsequent newlines
    // NB: maybe we should each repeating newlines inside the lexer
    // NB: making the lexer not eat new line enable the lexer to be closer
    // to the original code when it is used for formatting only
    void expect_newline(Node* wip_expression, CodeLocation const& loc);

    void expect_comment_or_newline(StmtNode* stmt, int depth, CodeLocation const& loc);

    void error_recovery(ParsingError* error);

    void ensure_valid();

    void show_diagnostics(std::ostream& out);

    // Shortcuts
    // ---------
    Token const& next_token();
    Token const& token() const { return _lex.token(); }
    Token const& peek_token() const { return _lex.peek_token(); }

    String get_identifier() const {
        if (token().type() == tok_identifier) {
            return token().identifier();
        }
        return String("<identifier>");
    }

    bool async() const {
        if (async_mode.size() <= 0) {
            return false;
        }

        return async_mode[int(async_mode.size()) - 1];
    }

    // not 100% sure this is the right way
    ExprContext context() const {
        if (_context.size() <= 0) {
            return ExprContext::Load;
        }

        return _context[int(_context.size()) - 1];
    }

    ParsingContext parsing_ctx() const {
        int n = int(parsing_context.size());
        if (n == 0)
            return ParsingContext::None;

        return parsing_context[n - 1];
    }

    bool is_tok_statement_ender() const;

    bool allow_slice() const { return parsing_ctx() == ParsingContext::Slice; }

    Array<ParsingError> const& get_errors() const { return errors; }

    enum class Mode
    {
        Stmt,
        Expr,
        Pattern
    };

    private:
    Token       previous = dummy();
    TokenBuffer currentline;

    public:
    int expression_depth = 0;

    void clear_errors() { errors.clear(); }

    private:
    Array<StmtNode*>      _pending_comments;
    bool                  with_extension = true;
    Array<ExprContext>    _context;
    Array<bool>           async_mode;
    Array<ParsingContext> parsing_context;
    AbstractLexer&        _lex;

    bool                is_empty_line = true;
    int                 current_error = -1;
    Array<ParsingError> errors;
};

}  // namespace lython
#endif
