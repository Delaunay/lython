#if __linux__
#    define __STDC_WANT_LIB_EXT1__   1
#    define __STDC_WANT_SECURE_LIB__ 1
#endif

#include <cstdio>
#include <cstdlib>

#include <filesystem>
#include "utilities/printing.h"

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "utilities/strings.h"
#include "dependencies/formatter.h"
#include "sema/importlib.h"


namespace lython {


ImportLib* ImportLib::instance() {
    static ImportLib self;
    return &self;
}

String internal_getenv(String const& name) {
    const char* envname = name.c_str();

#if (defined __STDC_LIB_EXT1__) || BUILD_WINDOWS
    size_t size = 0;

    getenv_s(&size, nullptr, 0, envname);

    // an error happened
    if (size == 0) {
        return {};
    }

    String path(size, ' ');
    int    err = getenv_s(&size, path.data(), path.size(), envname);

    if (err != 0) {
        return String();
    }
#else
    const char* value = getenv(envname);

    if (value == nullptr) {
        return String();
    }

    auto path = String(value);
#endif

    return path;
}

Array<String> python_paths() {
    String path = internal_getenv("PYTHONPATH");

    return split(':', path);
}

String is_module(Array<String> const& module_frags, String path, bool& is_mod) {
    kwdebug(outlog(), "looking in `{}`", path);

    namespace fs = std::filesystem;
    is_mod = false;

    auto stat = fs::status(path);
    if (!fs::is_directory(stat)) {
        kwdebug(outlog(), "Not a directory {}", path);
        return path;
    }

    Array<String> fspath_frags = {path};
    fspath_frags.reserve(module_frags.size());

    // <path>/<module_frags>
    std::copy(
        std::begin(module_frags),           //
        std::end(module_frags),             //
        std::back_inserter(fspath_frags)    //
    );

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
        kwdebug(outlog(), "not a file {}", fspath);
        return path;
    }

    kwdebug(outlog(), "Found file {}", fspath);
    is_mod = true;
    return fspath;
}


String ImportLib::lookup_module(StringRef const& module_path, Array<String> const& paths) {
    // First lookup for builtin module names
    // Then list of directories inside th sys.path
    //      sys.path contain 
    //          1. the directory of the input script or the current working directory
    //          2. PYTHONPATH
    //          3. Instalation Dependent default site-packages (this is handled by the site module)

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

    kwdebug(outlog(), "paths: {}", str(paths));
    auto module_frags = split('.', str(module_path));

     bool is_mod = false;

#if BUILD_WINDOWS
    // Windows use wchar, so we convert to a unicode string first
    const char* cpath = fs::current_path().u8string().c_str();
    String path = String(cpath);
#else
    String path = String(fs::current_path().c_str());
#endif

    auto path_mod = is_module(
        module_frags,                           //
        path,     //
        is_mod                                  //
    );

    if (is_mod) {
        return path_mod;
    }

    for (auto const& path: paths) {
        auto path_mod = is_module(module_frags, path, is_mod);
        if (is_mod) {
            return path_mod;
        }
    }

    return "";
}

ImportLib::ImportedLib* ImportLib::importfile(StringRef const& modulepath) {

    ImportedLib& importedlib = imported[modulepath];

    if (importedlib.mod == nullptr) {
        Module* mod = internal_importfile(modulepath, syspaths);

        if (mod != nullptr) {
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
            // TODO: this module also has init that will need to be called

            // Run sema on this module
            SemanticAnalyser* sema = new SemanticAnalyser(this);
            sema->exec(mod, 0);

            //
            if (sema->has_errors()) {
                // FIXME
            }
            //

            Dict<StringRef, ImportedLib>::iterator elem;
            bool ok = false;

            importedlib.mod = mod;
            importedlib.sema = sema;

            return &importedlib;
            // std::tie(elem, ok) = imported.insert({modulepath, ImportedLib{mod, sema}});
            // if (ok) {
            //     ImportedLib* val = &elem->second;
            //     return val;
            // }
            // kwwarn("Could not insert imported module to system");
        }

        kwwarn(outlog(), "Could not load file {}", modulepath);
        return nullptr;;
    }

    return &importedlib;
}

Module* ImportLib::internal_importfile(StringRef const& modulepath, Array<String> const& paths) {
    String filepath = lookup_module(modulepath, paths);
    if (filepath.empty()) {
        return nullptr;
    }

    FileBuffer buffer(filepath);
    Lexer      lexer(buffer);
    Parser     parser(lexer);
    Module*    mod = parser.parse_module();
    return mod;
}


void ImportLib::add_to_path(String const& path) {
    for (auto& other: syspaths) {
        if (path == other) {
            return;
        }
    }
    syspaths.push_back(path);
}


bool ImportLib::add_module(String const& name, Module* module) 
{
    SemanticAnalyser* sema = new SemanticAnalyser(this);
    sema->exec(module, 0);

    bool ok = false;
    std::tie(std::ignore, ok) = imported.insert({name, ImportedLib{module, sema}});

    if (ok) {
        return true;
    }

    return false;
}


Module* ImportLib::newmodule(String const& name) {
    modules.emplace_back(std::make_unique<Module>());
    UniquePtr<Module>& ptr = modules[int(modules.size()) - 1];
    // add_module(name, ptr.get());
    return ptr.get();
}

}