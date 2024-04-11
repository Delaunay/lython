#ifndef LYTHON_LOGGING_BACKTRACE_H
#define LYTHON_LOGGING_BACKTRACE_H

#include <cassert>
#include <string>
#include <vector>

#include "revision_data.h"

#include <fmt/core.h> 


namespace lython {



// Show backtrace using spdlog
void show_log_backtrace();

// Show backtrace using execinfo
void show_backtrace();

std::string demangle(std::string const& original_str);

// retrieve backtrace using execinfo
std::vector<std::string> get_backtrace(size_t size);


}  // namespace lython

#endif