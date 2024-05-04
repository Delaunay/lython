#include "parsing_error.h"

#include "ast/ops.h"
#include "lexer/lexer.h"
#include "lexer/unlex.h"
#include "utilities/printing.h"
#include "utilities/strings.h"

namespace lython {

ParsingError::ParsingError(Array<int> expected, Token token, Node* obj, CodeLocation loc):
    ParsingError(expected, token, loc) {
    switch (obj->family()) {
    case NodeFamily::Expression: expr = (ExprNode*)obj;
    case NodeFamily::Pattern: pat = (Pattern*)obj;
    case NodeFamily::Statement: stmt = (StmtNode*)obj;
    default: break;
    }
}

void add_wip_expr(ParsingError& err, StmtNode* stmt) { err.stmt = stmt; }

void add_wip_expr(ParsingError& err, ExprNode* expr) { err.expr = expr; }

void add_wip_expr(ParsingError& err, Node* expr) {
    switch (expr->family()) {
    case NodeFamily::Expression: err.expr = (ExprNode*)expr;
    case NodeFamily::Pattern: err.pat = (Pattern*)expr;
    case NodeFamily::Statement: err.stmt = (StmtNode*)expr;
    default: break;
    }
}

Node* get_expr(ParsingError const& error) {
    if (error.stmt) {
        return error.stmt;
    } else if (error.expr) {
        return error.expr;
    } else if (error.pat) {
        return error.pat;
    }
    return nullptr;
}

CommonAttributes* get_code_loc(ParsingError const& error) {
    if (error.stmt) {
        return error.stmt;
    } else if (error.expr) {
        return error.expr;
    } else if (error.pat) {
        return error.pat;
    }
    return nullptr;
}

String get_parent(ParsingError const& error) {
    if (error.stmt) {
        return shortprint(get_parent(error.stmt));
    } else if (error.expr) {
        return shortprint(get_parent(error.expr));
    } else if (error.pat) {
        return shortprint(get_parent(error.pat));
    }

    return "<module>";
}

String get_filename(ParsingErrorPrinter* printer) {
    if (printer->lexer) {
        return printer->lexer->file_name();
    }
    return "<input>";
}

void ParsingErrorPrinter::print_ast(ParsingError const& error,
                                    Node*               node,
                                    CommonAttributes*   srcloc) {
    // Print what we were able to parse
    {
        bool written = false;

        if (node) {
            auto         noline_buf = NoNewLine(out);
            std::ostream noline(&noline_buf);
            codeline();
            noline << str(node);
            noline.flush();
            written = true;
        }

        // Print the tokens that we were not able to parse
        if (error.remaining.size() > 0) {
            Unlex unlex;
            unlex.format(out, error.remaining);
            written = true;
        }

        if (written) {
            // Underline error if possible
            if (srcloc) {
                underline(*srcloc);
            } else {
                underline(error.received_token);
            }
        }
    }
}

void ParsingErrorPrinter::print_tok(ParsingError const& error, CommonAttributes* srcloc) {
    Unlex unlex;

    codeline();
    unlex.format(out, error.line);

    // Underline error if possible
    underline(error.received_token);
}

void ParsingErrorPrinter::print(ParsingError const& error) {
    String            filename = get_filename();
    Node*             node     = get_expr(error);
    CommonAttributes* srcloc   = get_code_loc(error);
    String            parent   = get_parent(error);

    int line = error.received_token.line();

    firstline() << "File \"" << filename << "\", line " << line << ", in " << parent;

// Lython own code loc
#if WITH_LOG
    if (with_compiler_code_loc) {
        newline() << error.loc.repr();
    }
#endif

    // Error message
    if (false) {
        print_ast(error, node, srcloc);
    } else {
        print_tok(error, srcloc);
    }

    errorline() << error.error_kind << ": ";
    if (error.expected_tokens.size() > 0) {
        Array<String> toks;
        std::transform(std::begin(error.expected_tokens),
                       std::end(error.expected_tokens),
                       std::back_inserter(toks),
                       [](int tok) -> String { return to_human_name(tok); });

        String expected = join("|", toks);

        out << "Expected: " << expected << " token but got " << to_human_name(error.received_token);
    } else {
        out << error.message;
    }

    end();
}

String shortprint(Node const* node) {
    if (node->kind == NodeKind::FunctionDef) {
        FunctionDef const* fun = static_cast<FunctionDef const*>(node);
        return str(fun->name);
    }
    if (node->kind == NodeKind::ClassDef) {
        ClassDef const* def = static_cast<ClassDef const*>(node);
        return str(def->name);
    }
    if (node->kind == NodeKind::Module) {
        return "<module>";
    }
    return str(node);
}

Node const* get_parent(Node const* parent) {
    Node const* n = parent->get_parent();
    Node const* p = parent;

    while (n != nullptr) {

        if (n->kind == NodeKind::Module) {
            return n;
        }

        if (n->kind == NodeKind::FunctionDef) {
            return n;
        }
        p = n;
        n = n->get_parent();
    }

    return p;
}

}  // namespace lython
