#include "logging.h"

#include <fmt/core.h> 
#include <fmt/format.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <iostream>

// Linux signal handling & stack trace printing
#ifdef __linux__
#    include <cxxabi.h>
#    include <execinfo.h>
#    include <regex>
#    include <signal.h>
#endif

namespace lython {
#ifdef __linux__

// _ZN6lython11builtin_maxERSt6vectorINS_5ValueENS_9AllocatorIS1_EEE+0x368
static std::regex mangled_name("\\([A-Za-z0-9_]*");
static std::regex remove_namespaces("(lython::|std::)");

std::string demangle(std::string const& original_str) {
    std::string matched_str;
    std::string result_str;

    auto begin = std::sregex_iterator(original_str.begin(), original_str.end(), mangled_name);
    auto end   = std::sregex_iterator();

    char* buffer = nullptr;

    int status = 0;
    for (auto i = begin; i != end; ++i) {
        matched_str = (*i).str();
        // ignore first (
        buffer = abi::__cxa_demangle(matched_str.c_str() + 1, nullptr, nullptr, &status);
        break;
    }

    if (!matched_str.empty() && status == 0) {
        result_str = std::string(buffer);
    } else {
        result_str = original_str;
    }

    free(buffer);
    return result_str;
}

std::vector<std::string> get_backtrace(size_t size = 32) {
    // avoid allocating memory dynamically
    std::vector<void*> ptrs(size);
    //    static std::vector<std::string> ignore = {
    //        "libstdc++.so",
    //        "lython::get_backtrace[abi:cxx11](unsigned long)",
    //        "lython::show_backkwtrace()"
    //    };

    int    real_size = backtrace(ptrs.data(), int(size));
    char** symbols   = backtrace_symbols(ptrs.data(), int(real_size));

    std::vector<std::string> names;
    names.reserve(size_t(real_size));

    for (int i = 0; i < real_size; ++i) {
        std::string original_str = demangle(symbols[i]);

        bool skip = false;
        if (original_str.find("libstdc++.so") != std::string::npos) {
            skip = true;
        }
        //        for (auto& str: ignore){
        //            if (original_str.find(str) != std::string::npos){
        //                skip = true;
        //            }
        //        }
        // skip libstdc++ calls
        if (!skip) {
            auto simplified = std::regex_replace(original_str, remove_namespaces, "");
            names.push_back(simplified);
        }
    }

    free(symbols);
    return names;
}

void show_backtrace() {
    std::vector<std::string> symbols = get_backtrace(32);
    int                      i       = 0;
    for (auto& sym: symbols) {
        i += 1;
        outlog().err(CodeLocation::noloc(), " TB {:2} -> {}", i, sym);
    }
}

[[noreturn]] void signal_handler(int sig) {
    outlog().fatal(CodeLocation::noloc(), "Received signal {} >>>", sig);
    show_backtrace();
    outlog().fatal(CodeLocation::noloc(), "<<< Exiting");
    exit(1);
}

int register_signal_handler() {
    signal(SIGSEGV, signal_handler);  // 11, install our handler
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    // Sent on exceptions which already print the stack trace
    signal(SIGABRT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);
    return 0;
}
#else
int register_signal_handler() { return 0; }

void show_backtrace() {}

std::vector<std::string> get_backtrace(size_t size) { return std::vector<std::string>(); }
#endif

void show_log_backtrace() {}




}  // namespace lython
