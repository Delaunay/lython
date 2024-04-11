#ifndef LYTHON_LOG_CODELOC_H
#define LYTHON_LOG_CODELOG_H

#include <cassert>
#include <string>
#include <vector>

#include "revision_data.h"

#include <fmt/core.h> 


namespace lython {

// Path to repository on current system
constexpr char __source_dir[] = _SOURCE_DIRECTORY;

// Length of the path so we can cut unimportant folders
constexpr int __size_src_dir = sizeof(_SOURCE_DIRECTORY) / sizeof(char);

struct CodeLocation {
    CodeLocation(std::string const& file,
                 std::string const& fun,
                 int                line,
                 std::string const& fun_long):
        filename(file.substr(__size_src_dir)),
        function_name(fun), line(line), function_long(fun_long) {}

    std::string filename;
    std::string function_name;
    int         line;
    std::string function_long;

    static CodeLocation const& noloc() {
        static CodeLocation loc(__FILE__, "", 0, "");
        return loc;
    }

    std::string repr() const { return fmt::format("{}:{} {}", filename, line, function_name); }
};

// Avoid spending time creating the same codeloc multiple time
// clang-format off
using CodeLocationConstRef = CodeLocation const&;

#define MAKE_LOC(fun, funp) lython::CodeLocation(__FILE__, fun, __LINE__, funp)
#define LAMBDA_LOC(fun, funp) (([](const char* afun, const char* afunp) -> lython::CodeLocationConstRef { static lython::CodeLocation loc = lython::CodeLocation(__FILE__, afun, __LINE__, afunp); return loc; })(fun, funp))
// clang-format on

#if WITH_LOG
#    ifdef __linux__
#        define LOC LAMBDA_LOC(__FUNCTION__, __PRETTY_FUNCTION__)
#    else
#        define LOC LAMBDA_LOC(__FUNCTION__, __func__)
#    endif
#else
#    define LOC lython::CodeLocation::noloc()
#endif

}

#endif