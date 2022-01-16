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

String lookup_module(StringRef const &module_path, Array<String> const &paths) {
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

        // TODO: check for so files
        //
        if (fs::is_directory(stat)) {
            // Load a folder module
            // import my.module => my/module/__init__.py
            fspath += "/__init__.py";
        } else {
            // Load a file module
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

    FileBuffer buffer(filepath);
    Lexer      lexer(buffer);
    Parser     parser(lexer);
    Module *   mod = parser.parse_module();
    return mod;
}

TypeExpr *SemanticAnalyser::import(Import *n, int depth) {
    // import datetime, time
    // import math as m
    for (auto &name: n->names) {
        StringRef nm  = name.name;
        auto      mod = process_file(name.name, paths);

        if (mod == nullptr) {
            SEMA_ERROR(ModuleNotFoundError(name.name));
            continue;
        }

        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        // Check ownership of Module
        // we could make the import statement the owner
        // but it could be imported multiple times
        // in that case we would like to avoid doing SEMA
        // and reuse the same version
        // we could also import modules using multiple threads
        // so we will need a place to manage all those modules
        // sounds like shared_ptr might the easiest
        // mod->move(n);

        // TODO: this needs to be kept somewhere
        SemanticAnalyser sema;
        sema.exec(mod, 0);

        bindings.add(nm, mod, lython::Module_t());
    }
    return nullptr;
}

StringRef get_name(ExprNode *target) {
    auto name = cast<Name>(target);
    if (!name) {
        return StringRef();
    }

    return name->id;
}

StmtNode *find(Array<StmtNode *> const &body, StringRef name) {
    for (auto stmt: body) {
        switch (stmt->kind) {
        case NodeKind::ClassDef: {
            auto def = cast<ClassDef>(stmt);
            if (def->name == name) {
                return stmt;
            }
            continue;
        }
        case NodeKind::FunctionDef: {
            auto def = cast<FunctionDef>(stmt);
            if (def->name == name) {
                return stmt;
            }
            continue;
        }
        case NodeKind::Assign: {
            auto ass = cast<Assign>(stmt);
            if (get_name(ass->targets[0]) == name) {
                return ass;
            }
            continue;
        }
        case NodeKind::AnnAssign: {
            auto ann = cast<AnnAssign>(stmt);
            if (get_name(ann->target) == name) {
                return ann;
            }
            continue;
        }
        default:
            continue;
        }
    }

    return nullptr;
}

TypeExpr *SemanticAnalyser::importfrom(ImportFrom *n, int depth) {
    Module *mod = nullptr;

    // Regular import using system path
    if (n->module.has_value() && !n->level.has_value()) {
        mod = process_file(n->module.value(), paths);

        if (mod == nullptr) {
            SEMA_ERROR(ModuleNotFoundError(n->module.value()));
            return nullptr;
        }
    } else if (n->level.has_value()) {
        // relative import using level

        if (mod == nullptr) {
            SEMA_ERROR(ModuleNotFoundError(n->module.value()));
            return nullptr;
        }
    }

    // TODO: Someone must be the owner of module
    // mod->move(n);

    // TODO: this needs to be kept somewhere
    SemanticAnalyser sema;
    sema.exec(mod, 0);

    for (auto &name: n->names) {
        StringRef nm = name.name;

        // lookup name.name inside module;
        // functions, classes are stmt
        // but variable could also be imported which are expressions
        StmtNode *value = find(mod->body, nm);

        if (value == nullptr) {
            debug("{} not found", nm);
            continue;
        }

        auto varid = sema.bindings.get_varid(nm);
        auto type  = sema.bindings.get_type(varid);

        // Did not find the value inside the module
        if (value == nullptr) {
            SEMA_ERROR(ImportError(n->module.value(), nm));
            continue;
        }

        //
        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        bindings.add(nm, value, type);
    }

    return nullptr;
}

} // namespace lython