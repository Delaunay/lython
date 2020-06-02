#include "ast/visitor.h"
#include "ast/print.h"
#include "parser/module.h"
#include "utilities/strings.h"

namespace lython {

#define TERMINAL_COLOR(COLOR)\
    COLOR(NC            , "\033[0m"   )\
    COLOR(WHITE         , "\033[1;37m")\
    COLOR(BLACK         , "\033[0;30m")\
    COLOR(BLUE          , "\033[0;34m")\
    COLOR(LIGHT_BLUE    , "\033[1;34m")\
    COLOR(GREEN         , "\033[0;32m")\
    COLOR(LIGHT_GREEN   , "\033[1;32m")\
    COLOR(CYAN          , "\033[0;36m")\
    COLOR(LIGHT_CYAN    , "\033[1;36m")\
    COLOR(RED           , "\033[0;31m")\
    COLOR(LIGHT_RED     , "\033[1;31m")\
    COLOR(PURPLE        , "\033[0;35m")\
    COLOR(LIGHT_PURPLE  , "\033[1;35m")\
    COLOR(BROWN         , "\033[0;33m")\
    COLOR(YELLOW        , "\033[1;33m")\
    COLOR(GRAY          , "\033[0;30m")\
    COLOR(LIGHT_GRAY    , "\033[1;30m")\

static String COLORS[] = {
#define COLOR(NAME, VAL) VAL,
    TERMINAL_COLOR(COLOR)
#undef COLOR
};

enum Colors{
#define COLOR(NAME, VAL) NAME,
    TERMINAL_COLOR(COLOR)
#undef COLOR
};

struct ASTPrinter: public ConstVisitor<ASTPrinter, std::ostream&>{
    std::ostream& out;
    int indent;
    int precedence = -1;
    bool debug_print;
    const bool colors;

    String const& color(Colors c){
        static String none = "";
        if (colors)
            return COLORS[int(c)];
        return none;
    }

#define KEYWORD(name)   color(PURPLE) << name << color(NC)
#define STATEMENT(name) color(YELLOW) << name << color(NC)
#define DOCSTRING(name) color(BROWN)  << name << color(NC)
#define DEBUG(name)     color(LIGHT_GRAY)   << name << color(NC)
#define DECL(name)      color(LIGHT_PURPLE) << name << color(NC)
#define TYPE            color(CYAN)
#define CALL            color(LIGHT_BLUE)
// everything is ref basically
// #define REF(name)       color(LIGHT_BLUE) << name << color(NC)
#define REF(name)       name

    ASTPrinter(std::ostream& out, int indent, bool debug_print, bool colors):
        out(out), indent(indent), debug_print(debug_print), colors(colors)
    {}

    std::ostream& undefined(Node_t, std::size_t){
        return out << "<undefined>";
    }

    std::ostream &parameter(Parameter_t param, std::size_t) {
        return out << param->name;
    }

    std::ostream &unary(UnaryOperator_t un, size_t d) {
        out << un->op;
        out << ' ';
        visit(un->expr, d);
        return out;
    }

    std::ostream& imported_expression(ImportedExpr_t expr, size_t){
        return out << "<import>";
    }

    std::ostream& import(Import_t imp, size_t){
        auto print_import = [&](AST::Import::DeclarationImport const& import){
            out << import.export_name;

            if (import.import_name && import.import_name != get_string("")){
                out << KEYWORD(" as ") << import.import_name;
            }
        };

        auto print_path = [&](AST::Import::PackagePath const& path){
            for (auto i = path.begin(); i != path.end() - 1; ++i){
                out << (*i) << ".";
            }
            out << *(path.end() - 1);
        };

        // from <> import <> as <>
        if (imp->imports.size()){
            out << KEYWORD("from ");
            print_path(imp->path);
            out << KEYWORD(" import ");

            size_t k = 0;
            for(auto& import: imp->imports){
                if (k + 1 < imp->imports.size()){
                    print_import(import);
                    out << ", ";
                }

                k += 1;
            }
            print_import(*(imp->imports.end() - 1));
        } else {
            out << KEYWORD("import ");
            print_path(imp->path);

            if (imp->name && imp->name != get_string("")){
                out << KEYWORD(" as ") << imp->name;
            }
        }

        return out;
    }

    std::ostream& match(Match_t mtch, size_t d){
        return out;
    }

    int get_precedence(Expression const& node){
        if (node.kind() == AST::NodeKind::KBinaryOperator){
            return node.ref<AST::BinaryOperator>()->precedence;
        }
        if (node.kind() == AST::NodeKind::KUnaryOperator){
            return node.ref<AST::UnaryOperator>()->precedence;
        }
        return -1;
    }

    std::ostream& loop(Loop_t loop, std::size_t){
        return out << "<loop>";
    }

    std::ostream &binary(BinaryOperator_t bin, std::size_t d) {
        bool parens = false;

        int prev = precedence;
        parens = prev >= bin->precedence;
        precedence = bin->precedence;

        if (parens)
            out << '(';

        visit(bin->lhs, d);

        String const& str = bin->op.str();

        if (str == "."){
            out << bin->op.str();
        } else {
            out << " " << bin->op.str() << " ";
        }

        visit(bin->rhs, d);

        if (parens)
            out << ')';

        precedence = prev;
        return out;
    }

    std::ostream &sequential(SeqBlock_t blocks, std::size_t d) {
        for (auto i = 0ul; i + 1 < blocks->blocks.size(); ++i) {
            out << String(std::size_t(indent) * 4, ' ');
            visit(blocks->blocks[i], d);
            out << '\n';
        }

        out << String(std::size_t(indent) * 4, ' ');
        visit(*(blocks->blocks.end() - 1), d);
        return out;
    }

    std::ostream &unparsed(UnparsedBlock_t blocks, std::size_t) {
        for (auto &tok : blocks->tokens)
            tok.print(out, indent);
        return out;
    }

    std::ostream &statement(Statement_t stmt, std::size_t d) {
        out << STATEMENT(keyword_as_string()[stmt->statement]) << " ";
        visit(stmt->expr, d);
        return out;
    }

    std::ostream &reference(Reference_t ref, std::size_t) {
        if (debug_print){
            return out << REF(ref->name) << DEBUG("[" << ref->index << "]");
        }
        return out << REF(ref->name);
    }

    std::ostream &builtin(Builtin_t blt, int32) {
        return out << blt->name;
    }

    std::ostream &type(Type_t type, std::size_t) {
        return out << type->name;
    }

    std::ostream &value(Value_t val, std::size_t d) {
        val->value.print(out);
        if (debug_print && val->type){
            out << ": " << TYPE;
            visit(val->type, d);
            out << color(NC);
        }
        return out;
    }

    std::ostream &struct_type(Struct_t cstruct, std::size_t d) {
        out << KEYWORD("struct ") << DECL(cstruct->name) << ":\n";
        std::string indentation = std::string(std::size_t((indent + 1) * 4), ' ');

        if (cstruct->docstring.size() > 0) {
            out << indentation << "\"\"\"" << cstruct->docstring << "\"\"\"\n";
        }

        //for (auto &item : cstruct->attributes) {
        for (auto i = 0ul; i + 1 < cstruct->attributes.size(); ++i) {
            auto const& item = cstruct->attributes[i];
            out << indentation << std::get<0>(item) << ": " << TYPE;
            visit(std::get<1>(item), d);
            out << color(NC) << "\n";
        }

        auto const& item = *(cstruct->attributes.end() - 1);
        out << indentation << std::get<0>(item) << ": " << TYPE;
        visit(std::get<1>(item), d);
        return out << color(NC);
    }

    std::ostream &call(Call_t call, std::size_t d) {
        out << CALL;
        visit(call->function, d);
        out << color(NC) << "(";

        int32 n = int32(call->arguments.size()) - 1;
        for (int i = 0; i < n; ++i) {
            visit(call->arguments[std::size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(call->arguments[std::size_t(n)], d);
        }

        return out << ")";
    }

    std::ostream &arrow(Arrow_t aw, std::size_t d) {
        int n = int(aw->params.size()) - 1;

        out << "(";

        for (int i = 0; i < n; ++i) {
            visit(&aw->params[size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(&aw->params[size_t(n)], d);
        }

        out << ") -> ";

        visit(aw->return_type, d);
        return out;
    }

    std::ostream& operator_fun(Operator_t op, std::size_t d){
        return out << op->name;
    }

    std::ostream& extern_function(ExternFunction_t extern_fun, std::size_t d){
        return out << extern_fun->name;
    }

    std::ostream &function(Function_t fun, std::size_t d) {
        // debug("Function Print");

        out << KEYWORD("def ") << DECL(fun->name) << "(";

        for (uint32 i = 0, n = uint32(fun->args.size()); i < n; ++i) {
            out << fun->args[i].name;

            if (fun->args[i].type) {
                out << ": " << TYPE;
                visit(fun->args[i].type, d);
                out << color(NC);
            }

            if (i < n - 1)
                out << ", ";
        }

        if (fun->return_type) {
            out << ") -> " << TYPE;
            visit(fun->return_type, d);
            out << color(NC) << ":\n";
        } else
            out << "):\n";

        std::string indentation = std::string(size_t(indent + 1) * 4, ' ');

        if (fun->docstring.size() > 0) {
            out << indentation << "\"\"\"" << fun->docstring << "\"\"\"\n";
        }

        indent += 1;
        visit(fun->body, d);
        indent -= 1;
        return out;
    }
};

std::ostream& print(std::ostream& out, const Expression expr, int indent, bool dbg, bool colors){
    return ASTPrinter(out, indent, dbg, colors).visit(expr);
}
}
