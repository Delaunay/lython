#pragma once


#include "dtypes.h"
#include "ast/nodes.h"
#include "utilities/names.h"

namespace lython {

Array<String> python_paths();

StmtNode* find(Array<StmtNode*> const& body, StringRef const& name);

// Unique instance managing all the imported module
// Currently it only lookup modules and cache the parsing
//
// This is a singleton for convenience but SEMA should be able to take any instance
// maybe this should become the owner of all the modules
//
// it will also be the sync point if modules are parser + sema in parallel
class ImportLib 
{
public:
    struct ImportedLib {
        Module* mod = nullptr;
        struct SemanticAnalyser* sema = nullptr;
    };

    ImportedLib* importfile(StringRef const& modulepath);

    static ImportLib* instance();

    void add_to_path(String const& path);

    // Note: native module still go through SEMA
    // although it is quite a simple pass because native modules
    // should only have Builtin Nodes
    bool add_module(String const& name, Module* module);

    Module* newmodule(String const& name);

private:

    String lookup_module(StringRef const& module_path, Array<String> const& paths);

    Module* internal_importfile(StringRef const& modulepath, Array<String> const& paths);

    Dict<StringRef, ImportedLib> imported;

    Array<String> syspaths = python_paths();

    Array<UniquePtr<Module>> modules;
};

}