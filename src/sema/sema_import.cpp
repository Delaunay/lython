#if __linux__
#    define __STDC_WANT_LIB_EXT1__   1
#    define __STDC_WANT_SECURE_LIB__ 1
#endif

#include <cstdio>
#include <cstdlib>

#include <filesystem>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"

#include "dependencies/formatter.h"

// getenv(name)
// setenv(name, value, override)
// unsetenv(name)

namespace lython {



TypeExpr* SemanticAnalyser::import(Import* n, int depth) {
    // import datetime, time
    // import math as m
    for (auto& name: n->names) {
        StringRef nm  = name.name;
        auto*     imported = importsys->importfile(name.name);

        if (imported == nullptr || imported->mod == nullptr) {
            SEMA_ERROR(n, ModuleNotFoundError, name.name);
            continue;
        }

        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        bindings.add(nm, imported->mod, lython::Module_t());
    }
    return nullptr;
}

StringRef get_name(ExprNode* target) {
    auto* name = cast<Name>(target);
    if (name == nullptr) {
        return StringRef();
    }

    return name->id;
}

StmtNode* find(Array<StmtNode*> const& body, StringRef const& name) {
    for (auto* stmt: body) {
        switch (stmt->kind) {
        case NodeKind::ClassDef: {
            auto* def = cast<ClassDef>(stmt);
            if (def->name == name) {
                return stmt;
            }
            continue;
        }
        case NodeKind::FunctionDef: {
            auto* def = cast<FunctionDef>(stmt);
            if (def->name == name) {
                return stmt;
            }
            continue;
        }
        case NodeKind::Assign: {
            auto* ass = cast<Assign>(stmt);
            if (get_name(ass->targets[0]) == name) {
                return ass;
            }
            continue;
        }
        case NodeKind::AnnAssign: {
            auto* ann = cast<AnnAssign>(stmt);
            if (get_name(ann->target) == name) {
                return ann;
            }
            continue;
        }
        default: continue;
        }
    }

    return nullptr;
}

TypeExpr* SemanticAnalyser::importfrom(ImportFrom* n, int depth) {
    // Module* mod = nullptr;
    ImportLib::ImportedLib* imported = nullptr;

    // Regular import using system path
    if (n->module.has_value() && !n->level.has_value()) {
        imported = importsys->importfile(n->module.value());

        if (imported == nullptr || imported->mod == nullptr) {
            SEMA_ERROR(n, ModuleNotFoundError, n->module.value());
            return nullptr;
        }
    } else if (n->level.has_value()) {
        // relative import using level

        if (imported == nullptr) {
            SEMA_ERROR(n, ModuleNotFoundError, n->module.value());
            return nullptr;
        }
    }

    for (auto& name: n->names) {
        StringRef nm = name.name;

        // lookup name.name inside module;
        // functions, classes are stmt
        // but variable could also be imported which are expressions
        StmtNode* value = find(imported->mod->body, nm);

        if (value == nullptr) {
            kwdebug(outlog(), "{} not found", nm);
            continue;
        }

        Bindings& import_bindings = imported->sema->bindings;

        //auto  varid = import_bindings.get_varid(nm);
        //auto* type  = import_bindings.get_type(varid);

        // Did not find the value inside the module
        if (value == nullptr) {
            SEMA_ERROR(n, ImportError, n->module.value(), nm);
            continue;
        }

        //
        if (name.asname.has_value()) {
            nm = name.asname.value();
        }

        #if 0
            Exported* e_value = n->new_object<Exported>();
            e_value->source = &import_bindings;
            e_value->dest = &bindings;
            e_value->node = value;

            Exported* e_type = n->new_object<Exported>();
            e_type->source = &import_bindings;
            e_type->dest = &bindings;
            e_type->node = type;

            bindings.add(nm, e_value, e_type);
        #else
            // type attached to this value might 
            // not have the right var id
            ExprNode* type = nullptr;
            if (BindingEntry* entry = import_bindings.find(nm)) {
                type = entry->type;
            }
            bindings.add(nm, value, type);
        #endif
    }

    return nullptr;
}

}  // namespace lython