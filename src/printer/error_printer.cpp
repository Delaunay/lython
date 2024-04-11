#include "error_printer.h"
#include "ast/nodes.h"
#include "lexer/lexer.h"

namespace lython {

String        BaseErrorPrinter::indentation() { return String(indent * 2, ' '); }
std::ostream& BaseErrorPrinter::firstline() { return out; }
std::ostream& BaseErrorPrinter::newline() { return out << std::endl << indentation(); }
std::ostream& BaseErrorPrinter::errorline() { return out << std::endl; }
void          BaseErrorPrinter::end() { out << std::endl; }
std::ostream& BaseErrorPrinter::codeline() {
    return out << std::endl << indentation() << indentation() << "|";
}

void BaseErrorPrinter::underline(CommonAttributes const& attr) {

    int32 size = 1;

    if (attr.end_col_offset.has_value()) {
        size = std::max(attr.end_col_offset.value() - attr.col_offset, 1);
    }

    int32 start = std::max(1, attr.col_offset);
    codeline() << String(start, ' ') << String(size, '^');
}

String BaseErrorPrinter::get_filename() const {
    if (lexer != nullptr) {
        return lexer->file_name();
    }
    return "<input>";
}

StmtNode* get_parent_stmt(Node* node) {
    Array<Node const*> nodes;
    Node const*        n = node;

    while (n != nullptr) {
        if (n->family() == NodeFamily::Statement) {
            return (StmtNode*)n;
        }
        if (n->family() == NodeFamily::Module) {
            return nullptr;
        }
        n = n->get_parent();

        for (auto const* prev: nodes) {
            if (prev == n) {
                kwerror(outlog(), "Circle found inside GC nodes");
                // circle should not happen
                return nullptr;
            }
        }
        nodes.push_back(n);
    }
    return nullptr;
}

void BaseErrorPrinter::underline(Token const& tok) {
    int32 tok_size  = std::max(1, tok.end_col() - tok.begin_col());
    int32 tok_start = std::max(1, tok.begin_col());
    codeline() << String(tok_start, ' ') << String(tok_size, '^');
}

}  // namespace lython