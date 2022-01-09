#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"

namespace lython {

String lookup_module(StringRef const &path, Array<String> const &paths) {
    // Look for the module in the path
    // env/3.9.7/lib/python39.zip
    // env/3.9.7/lib/python3.9
    // env/3.9.7/lib/python3.9/lib-dynload
    // env/3.9.7/lib/python3.9/site-packages
    // handle PTH files
    // add the module to the context

    // Check current directory for the module
    // Check the path from first to last
    return "";
}

Module *process_file(StringRef const &modulepath, Array<String> const &paths) {
    String           filepath = lookup_module(modulepath, paths);
    FileBuffer       buffer(filepath);
    Lexer            lexer(buffer);
    Parser           parser(lexer);
    Module *         mod = parser.parse_module();
    SemanticAnalyser sema;
    sema.exec(mod, 0);
    return mod;
}

TypeExpr *SemanticAnalyser::import(Import *n, int depth) {
    // import datetime, time
    // import math as m
    for (auto &name: n->names) {
        StringRef nm  = name.name;
        auto      mod = process_file(name.name, paths);

        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        bindings.add(nm, mod, lython::Module_t());
    }
    return nullptr;
}

TypeExpr *SemanticAnalyser::importfrom(ImportFrom *n, int depth) {
    Module *mod = nullptr;

    if (n->module.has_value()) {
        auto mod = process_file(n->module.value(), paths);
    } else if (n->level.has_value()) {
        // relative import using level
    }

    if (mod == nullptr) {
        // ImportError: Module not found
        return nullptr;
    }

    for (auto &name: n->names) {
        StringRef nm = name.name;

        // lookup name.name inside module;
        // functions, classes are stmt
        // but variable could also be imported which are expressions
        StmtNode *value = nullptr;

        //
        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        bindings.add(nm, value, lython::Module_t());
    }

    return nullptr;
}

} // namespace lython