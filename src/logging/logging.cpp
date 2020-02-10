#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logging.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <unordered_map>
#include <memory>

// Linux signal handling & stack trace printing
#ifdef __linux__
#include <execinfo.h>
#include <signal.h>
#include <cxxabi.h>
#include <regex>
#endif

namespace lython {
#ifdef __linux__

// _ZN6lython11builtin_maxERSt6vectorINS_5ValueENS_9AllocatorIS1_EEE+0x368
static std::regex mangled_name("\\([A-Za-z0-9_]*");


std::string demangle(std::string const& original_str){
    std::string matched_str;
    std::string result_str;

    auto begin = std::sregex_iterator(original_str.begin(), original_str.end(), mangled_name);
    auto end = std::sregex_iterator();

    size_t size = original_str.size() - 1;
    char* buffer = nullptr;

    int status = 0;
    for(auto i = begin; i != end;){
        matched_str = (*i).str();
        // ignore first (
        buffer = abi::__cxa_demangle(matched_str.c_str() + 1, nullptr, nullptr, &status);
        break;
    }

    if (matched_str.size() > 0 && status == 0){
        result_str = std::string(buffer);
    } else {
        result_str = original_str;
    }

    free(buffer);
    return result_str;
}

std::vector<std::string> get_backtrace(size_t size=32){
    // avoid allocating memory dynamically
    std::vector<void*> ptrs(size);
//    static std::vector<std::string> ignore = {
//        "libstdc++.so",
//        "lython::get_backtrace[abi:cxx11](unsigned long)",
//        "lython::show_backtrace()"
//    };

    int real_size = backtrace(ptrs.data(), int(size));
    char** symbols = backtrace_symbols(ptrs.data(), int(real_size));

    std::vector<std::string> names;
    names.reserve(size_t(real_size));

    for (int i = 0; i < real_size; ++i){
        std::string original_str = demangle(symbols[i]);

        bool skip = false;
        if (original_str.find("libstdc++.so") != std::string::npos){
            skip = true;
        }
//        for (auto& str: ignore){
//            if (original_str.find(str) != std::string::npos){
//                skip = true;
//            }
//        }
        // skip libstdc++ calls
        if (!skip){
            names.push_back(original_str);
        }
    }

    free(symbols);
    return names;
}

void show_backtrace() {
    std::vector<std::string> symbols = get_backtrace(16);
    for (auto& sym: symbols){
        spdlog_log(LogLevel::Fatal, sym);
    }
}

void signal_handler(int sig){
    spdlog_log(LogLevel::Fatal, fmt::format("Received signal {} >>>", sig));
    show_backtrace();
    spdlog_log(LogLevel::Fatal, "<<< Exiting");
    exit(1);
}

int register_signal_handler(){
    signal(SIGSEGV, signal_handler);   // install our handler
//    signal(SIGINT, signal_handler);
//    signal(SIGQUIT, signal_handler);
//    signal(SIGABRT, signal_handler);
//    signal(SIGKILL, signal_handler);
//    signal(SIGTERM, signal_handler);
    return 0;
}
#else
void register_signal_handler(){}

void show_backtrace(){}

std::vector<std::string> get_backtrace(size_t size){
    return std::vector<std::string>();
}
#endif

using Logger = std::shared_ptr<spdlog::logger>;

Logger new_logger(char const* name){
    // Static so only executed once
    static int _ = register_signal_handler();

    spdlog::enable_backtrace(32);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto console = std::make_shared<spdlog::logger>(name, stdout_sink);

    console->set_level(spdlog::level::level_enum::trace);
    console->flush_on(spdlog::level::level_enum::trace);

    spdlog::register_logger(console);
    spdlog::set_pattern("[%L] [%d-%m-%Y %H:%M:%S.%e] [%t] %v");

    return console;
}


Logger root(){
    static Logger log = new_logger("root");
    return log;
}


static constexpr spdlog::level::level_enum log_level_spd[] = {
    spdlog::level::level_enum::trace,
    spdlog::level::level_enum::debug,
    spdlog::level::level_enum::info,
    spdlog::level::level_enum::warn,
    spdlog::level::level_enum::err,
    spdlog::level::level_enum::critical,
    spdlog::level::level_enum::off
};

void show_log_backtrace(){
    spdlog::dump_backtrace();
}


void spdlog_log(LogLevel level, std::string const& msg){
    root()->log(log_level_spd[level], msg);
}

const char* log_level_str[] = {
    "[T] TRACE",
    "[D] DEBUG",
    "[I]  INFO",
    "/!\\  WARN",
    "[E] ERROR",
    "[!] FATAL",
    ""
};

std::string format_code_loc(const char* file, const char* function, int line){
    return fmt::format(
        "{} {}:{}", file, function, line);
}


std::string format_code_loc_trace(const char*, const char* function, int line){
    return fmt::format(
        "{:>25}:{:4}", function, line);
}


// instead of setting a single log level for the entire program allow to cherry pick
// which level is enabled
std::unordered_map<LogLevel, bool>& log_levels(){
    static std::unordered_map<LogLevel, bool> levels{
        {Info,  true},
        {Warn,  true},
        {Debug, true},
        {Error, true},
        {Fatal, true},
        {Trace, true}
    };
    return levels;
}

void set_log_level(LogLevel level, bool enabled){
    log_levels()[level] = enabled;
}

bool is_log_enabled(LogLevel level){
    return log_levels()[level];
}

const char* Exception::what() const noexcept {
    show_backtrace();
    return message.c_str();
}

}
