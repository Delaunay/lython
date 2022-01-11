#include <cstdlib>
#include <filesystem>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"

// getenv(name)
// setenv(name, value, override)
// unsetenv(name)

namespace lython {

Array<String> python_paths() {
    const char *value = getenv("PYTHONPATH");
    if (value == nullptr) {
        return {};
    }
    auto path = String(value);
    return split(':', path);
}

String lookup_module(StringRef const &module_path, Array<String> const &additional_paths) {
    // Look for the module in the path
    // env/3.9.7/lib/python39.zip
    // env/3.9.7/lib/python3.9
    // env/3.9.7/lib/python3.9/lib-dynload
    // env/3.9.7/lib/python3.9/site-packages
    // handle PTH files
    // add the module to the context

    // Check current directory for the module
    // Check the path from first to last

    namespace fs = std::filesystem;
    auto paths   = python_paths();

    debug("{}", str(paths));
    auto module_frags = split('.', str(module_path));

    for (auto path: paths) {
        auto stat = fs::status(path);
        if (!fs::is_directory(stat)) {
            debug("Not a directory {}", path);
            continue;
        }

        Array<String> fspath_frags = {path};
        fspath_frags.reserve(module_frags.size());

        // <path>/<module_frags>
        std::copy(std::begin(module_frags), std::end(module_frags),
                  std::back_inserter(fspath_frags));

        auto fspath = join("/", fspath_frags);
        stat        = fs::status(fspath);

        // Load a folder module
        if (fs::is_directory(stat)) {
            // import my.module => my/module/__init__.py
            fspath += "/__init__.py";
        } else {
            // import my.module => my/module.py
            fspath += ".py";
        }

        stat = fs::status(fspath);
        if (!fs::exists(stat)) {
            debug("not a file {}", fspath);
            continue;
        }

        debug("Found file {}", fspath);
        return fspath;
    }

    return "";
}

Module *process_file(StringRef const &modulepath, Array<String> const &paths) {
    String filepath = lookup_module(modulepath, paths);
    if (filepath == "") {
        return nullptr;
    }

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