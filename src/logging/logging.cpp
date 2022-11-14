#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "logging.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
#include <unordered_map>

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
    //        "lython::show_backtrace()"
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
        spdlog_log(LogLevel::Error, fmt::format(" TB {:2} -> {}", i, sym));
    }
}

[[noreturn]] void signal_handler(int sig) {
    spdlog_log(LogLevel::Fatal, fmt::format("Received signal {} >>>", sig));
    show_backtrace();
    spdlog_log(LogLevel::Fatal, "<<< Exiting");
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

using Logger = std::shared_ptr<spdlog::logger>;

Logger new_logger(char const* name) {
    // Static so only executed once
    static int _ = register_signal_handler();

    spdlog::enable_backtrace(32);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto console = std::make_shared<spdlog::logger>(name, stdout_sink);

    console->set_level(spdlog::level::level_enum::trace);
    console->flush_on(spdlog::level::level_enum::trace);

    spdlog::register_logger(console);
    // %Y-%m-%d %H:%M:%S.%e
    spdlog::set_pattern("[%L] [%t] %v");

    return console;
}

Logger new_ostream_logger(char const* name, std::ostream& out) {
    auto ossink  = std::make_shared<spdlog::sinks::ostream_sink_st>(out);
    auto console = std::make_shared<spdlog::logger>(name, ossink);

    console->set_level(spdlog::level::level_enum::trace);
    console->flush_on(spdlog::level::level_enum::trace);

    spdlog::register_logger(console);
    // %Y-%m-%d %H:%M:%S.%e
    spdlog::set_pattern("[%L] [%t] %v");
    return console;
}

std::unordered_map<const char*, Logger>& logger_handles() {
    static std::unordered_map<const char*, Logger> handles;
    return handles;
}

LoggerHandle new_log(const char* name, std::ostream& out) {
    Logger log             = new_ostream_logger(name, out);
    logger_handles()[name] = log;
    return (LoggerHandle)(log.get());
}

Logger root() {
    static Logger log = new_logger("root");
    return log;
}

static constexpr spdlog::level::level_enum log_level_spd[] = {spdlog::level::level_enum::trace,
                                                              spdlog::level::level_enum::debug,
                                                              spdlog::level::level_enum::info,
                                                              spdlog::level::level_enum::warn,
                                                              spdlog::level::level_enum::err,
                                                              spdlog::level::level_enum::critical,
                                                              spdlog::level::level_enum::off};

void show_log_backtrace() { spdlog::dump_backtrace(); }

void spdlog_log(LogLevel level, std::string const& msg) { root()->log(log_level_spd[level], msg); }

const char* log_level_str[] = {
    "[T] TRACE", "[D] DEBUG", "[I]  INFO", "/!\\  WARN", "[E] ERROR", "[!] FATAL", ""};

std::string format_code_loc(const char* file, const char* function, int line) {
    return fmt::format("{} {}:{}", file, function, line);
}

std::string format_code_loc_trace(const char*, const char* function, int line) {
    return fmt::format("{:>25}:{:4}", function, line);
}

std::string format_function(std::string const& fun) {
    auto start = fun.find_last_of(':');
    if (start == std::string::npos) {
        return fun;
    }
    return fun.substr(start + 1);
}

// instead of setting a single log level for the entire program allow to cherry pick
// which level is enabled
std::unordered_map<LogLevel, bool>& log_levels() {
    static std::unordered_map<LogLevel, bool> levels{
        {Info, true},
        {Warn, true},
        {Debug, true},
        {Error, true},
        {Fatal, true},
        {Trace, true},
    };
    return levels;
}

void set_log_level(LogLevel level, bool enabled) { log_levels()[level] = enabled; }

bool is_log_enabled(LogLevel level) { return log_levels()[level]; }

}  // namespace lython
