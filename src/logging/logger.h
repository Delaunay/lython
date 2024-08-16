#ifndef LYTHON_LOG_LOGGER_H
#define LYTHON_LOG_LOGGER_H

#include <cassert>
#include <string>
#include <iostream>
#include <vector>

#include "revision_data.h"
#include "logging/codeloc.h"
#include "kmeta.h"

#include <fmt/core.h> 
#include <fmt/format.h>

namespace lython {

enum class LogLevel {
    Fatal,
    Error,
    Warn,
    Info,
    Debug,
    Trace,
    All
};

std::string const& logshort(LogLevel level);

// remove namespace info
std::string format_function(std::string const&);

// using FormatBuffer = std::vector<char>;
using FormatBuffer = fmt::memory_buffer;

struct KIGNORE() Output {
    virtual ~Output() {}
    virtual void write(FormatBuffer const& msg) = 0;
    virtual void write(std::string const& msg) = 0;

    virtual void flush_to(Output& out) {}
    virtual void flush() {}
    virtual void clear() {}
};


struct KIGNORE() Stdout: public Output {
    void write(FormatBuffer const& msg) override;
    void write(std::string const& msg) override;
};


struct KIGNORE() Stderr: public Output {
    void write(FormatBuffer const& msg) override;
    void write(std::string const& msg) override;
};


struct KIGNORE() FileOut: public Output {
    void write(FormatBuffer const& msg) override;
    void write(std::string const& msg) override;
};


struct KIGNORE() InMemory: public Output {
    void write(FormatBuffer const& msg) override;
    void write(std::string const& msg) override;
    void flush_to(Output& out) override;

    void clear() override {
        entries.clear();
    }

    std::vector<std::string> entries;
};


struct OutputProxy {
    int i = 0;
};

struct KIGNORE() Logger {
    Logger(std::string const& name):
        name(name)
    {
        buffer = FormatBuffer();
    }

    template <typename... Args>
    void log(LogLevel level, CodeLocation const& loc,  fmt::string_view fmtstr, const Args&... args) {
        // [I] logname filename:line fun - msg
        if (is_enabled(level)) {
            buffer.clear();
            buffer.reserve(fmt::formatted_size(fmtstr, args...));
            fmt::format_to(
                std::back_inserter(buffer), 
                fmt::format("{}{}", "{} {} {}:{} {} - ", fmtstr), 
                logshort(level),
                name,
                loc.filename,
                loc.line,
                format_function(loc.function_name),
                args...
            );
            push(buffer);
            buffer.clear();
        }
    }

    template <bool end, typename... Args>
    void logtrace(LogLevel level, CodeLocation const& loc, int depth, fmt::string_view fmtstr, const Args&... args) {
        if (is_enabled(level)) {
            // [I] logname fun:line :|:|+-> msg
            // [I] logname fun:line :|:|+-< msg
            fmt::string_view base = "{} {} {:>30}:{:4} {}+-> ";
            if (end) {
                base = "{} {} {:>30}:{:4} {}+-< ";
            }

            std::string depthstr(depth, ' ');
            for (size_t i = 0; i < depth; ++i) {
                depthstr[i] = i % 2 ? '|' : ':';
            }
                        
            fmt::format_to(
                std::back_inserter(buffer), 
                fmt::format("{}{}", base, fmtstr), 
                logshort(level),
                name,
                format_function(loc.function_name),
                loc.line,
                depthstr,
                args...
            );
            push(buffer);
            buffer.clear();
        }
    }

    #define SHORTCUT(name, level)                                                       \
        template <typename... Args>                                                     \
        void name(Args ...args) { log(LogLevel::level, args...); }                      \
        template <bool end, typename... Args>                                           \
        void name ## _ ## trace(Args ...args) { logtrace<end>(LogLevel::level, args...); }

    SHORTCUT(fatal, Fatal)
    SHORTCUT(err, Error)
    SHORTCUT(warn, Warn)
    SHORTCUT(info, Info)
    SHORTCUT(debug, Debug)
    SHORTCUT(trace, Trace)
    #undef SHORTCUT
    void add_output(OutputProxy proxy);
    void push(FormatBuffer const& msg) ;
    void push(std::string const& msg) ;
    void verbosity(LogLevel level);
    void disable_all();
    void enable_all();
    bool is_enabled(LogLevel level) const ;
    void enable(LogLevel level) ;
    void disable(LogLevel level);

    std::string const name;
    std::vector<int> outputs;
private:
    std::uint64_t    levels = (~0);
    FormatBuffer     buffer;
};

struct LogSystem {
    std::vector<std::unique_ptr<Output>> outputs;
    std::vector<std::unique_ptr<Logger>> loggers;

    LogSystem();

    static LogSystem& system() {
        static LogSystem sys;
        return sys;
    }
};


template<typename T, typename ...Args>
OutputProxy new_output(Args... args) {
    auto& outputs = LogSystem::system().outputs;
    int i = int(outputs.size());
    outputs.push_back(std::make_unique<T>(args...));
    return OutputProxy{i};
}

Logger& new_log(std::string const& name);

Logger& new_stdout_log(std::string const& name);
Logger& new_stderr_log(std::string const& name);

Logger& outlog();
Logger& errlog();

inline void assert_true(bool                        cond,
                        char const*                 message,
                        char const*                 assert_expr,
                        lython::CodeLocation const& loc) {
    if (!cond) {
        lython::outlog().log(lython::LogLevel::Error,
                    loc,
                    "Assertion error: {}\n"
                    "  - expr: {}",
                    message,
                    assert_expr);

        // lython::show_backkwtrace();
        assert(cond);
    }
}

#undef SHORTCUT

#define SHORTCUT(name, level)                                                           \
    template <typename... Args>                                                         \
    void name(Logger& logger, Args ...args) { logger.log(LogLevel::level, args...); }   \
    template <bool end, typename... Args>                                               \
    void name ## _ ## trace(Logger& logger, Args ...args) { logger.logtrace<end>(LogLevel::level, args...); }

SHORTCUT(fatal, Fatal)
SHORTCUT(err, Error)
SHORTCUT(warn, Warn)
SHORTCUT(info, Info)
SHORTCUT(debug, Debug)
SHORTCUT(trace, Trace)
#undef SHORTCUT
}

#endif