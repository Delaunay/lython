#include <iostream>
#include <cassert>

#include "logger.h"


namespace lython {

 void Stdout::write(FormatBuffer const& msg) {
    std::cout << std::string(msg.data(), msg.size()) << std::endl;
 }
  void Stderr::write(FormatBuffer const& msg) {
    std::cerr << std::string(msg.data(), msg.size()) << std::endl;
 }

 void FileOut::write(FormatBuffer const& msg) {}

 void InMemory::write(FormatBuffer const& msg) {
    entries.emplace_back(msg.data(), msg.size());
 }

void Stdout::write(std::string const& msg) {
    std::cout << std::string(msg) << std::endl;
 }
void Stderr::write(std::string const& msg) {
    std::cerr << std::string(msg) << std::endl;
 }

 void FileOut::write(std::string const& msg) {}

 void InMemory::write(std::string const& msg) {
    entries.emplace_back(msg);
 }

void InMemory::flush_to(Output& out) {
    for(auto& entry: entries) {
        out.write(entry);
    }
}



void Logger::add_output(OutputProxy proxy) {
    outputs.push_back(proxy.i);
}

void Logger::push(std::string const& msg) {
    auto& outs = LogSystem::system().outputs;
    for(int proxy: outputs) {
        outs[proxy]->write(msg);
    }
}

void Logger::push(FormatBuffer const& msg) {
    auto& outs = LogSystem::system().outputs;
    for(int proxy: outputs) {
        outs[proxy]->write(msg);
    }
}

void Logger::verbosity(LogLevel level) {
    for(int i = 0; i < int(LogLevel::All); i++) {
        if (int(level) < i) {
            enable(LogLevel(i));
        } else {
            disable(LogLevel(i));
        }
    }
}

void Logger::disable_all() {
    levels = 0;
}

void Logger::enable_all() {
    levels = ~0;
}

bool Logger::is_enabled(LogLevel level) const {
    return levels & (std::uint64_t(1) << std::uint64_t(level));
}

void Logger::enable(LogLevel level) {
    std::uint64_t ilevel = std::uint64_t(level);
    levels = levels | (std::uint64_t(1) << ilevel); 
}

void Logger::disable(LogLevel level) {
    std::uint64_t ilevel = std::uint64_t(level);
    levels = levels & ~(std::uint64_t(1) << ilevel);
}


const char* log_level_str[] = {
    "[T] TRACE", "[D] DEBUG", "[I]  INFO", "/!\\  WARN", "[E] ERROR", "[!] FATAL", ""};

std::string log_short[6] = {
    "[!]",
    "[E]",
    "[W]",
    "[I]",
    "[D]",
    "[T]"
};

std::string const& logshort(LogLevel level) {
    assert(int(level) < 6);
    return log_short[int(level)];
}

std::string format_code_loc(const char* file, const char* function, int line) {
    return fmt::format("{} {}:{}", file, function, line);
}

std::string format_code_loc_kwtrace(const char*, const char* function, int line) {
    return fmt::format("{:>25}:{:4}", function, line);
}

std::string format_function(std::string const& fun) {
    auto start = fun.find_last_of(':');
    if (start == std::string::npos) {
        return fun;
    }
    return fun.substr(start + 1);
}

LogSystem::LogSystem() {
    outputs.push_back(std::make_unique<Stdout>());
    outputs.push_back(std::make_unique<Stderr>());
}

Logger& new_log(std::string const& name) {
    auto& loggers = LogSystem::system().loggers;
    for(auto& log: loggers) {
        if (log->name == name) {
            return *log.get();
        }
    }
    int i = int(loggers.size());
    loggers.push_back(std::make_unique<Logger>(name));
    return new_log(name);
}

Logger& new_stdout_log(std::string const& name) {
    Logger& log = new_log(name);
    log.add_output(OutputProxy{0});
    log.enable_all();
    return log;
}

Logger& new_stderr_log(std::string const& name) {
    Logger& log = new_log(name);
    log.add_output(OutputProxy{1});
    log.enable_all();
    return log;
}

Logger& outlog() {
    static Logger& log = new_stdout_log("stdout");
    return log;
}

Logger& errlog() {
    static Logger& log = new_stdout_log("stderr");
    return log;
}

}