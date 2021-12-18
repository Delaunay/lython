#include "logging/logging.h"
#include "names.h"

namespace lython {

std::ostream &operator<<(std::ostream &out, StringRef ref) {
    return out << StringDatabase::instance()[ref.__id__()];
}

void StringRef::print(std::ostream &out) const {
    auto view = StringDatabase::instance()[ref];
    out << String(view);
}

StringRef::operator StringView() const { return StringDatabase::instance()[ref]; }

StringRef::~StringRef() { StringDatabase::instance().dec(ref); }

std::ostream &StringDatabase::report(std::ostream &out) const {
    std::size_t saved    = 0;
    std::size_t saved_up = 0;
    std::size_t size     = 9;

    out << fmt::format("| {:30} | {:4} | {:4} | {:4} | {:4} | {:4} |\n", "str", "#", "use", "cpy",
                       "low", "upp");

    for (auto &entry: strings) {
        size += entry.data.size();
        auto lower = entry.data.size() * (entry.count - 1);
        auto upper = entry.data.size() * (entry.copy - 1);

        out << fmt::format("| {:30} | {:4} | {:4} | {:4} | {:4} | {:4} |\n", entry.data,
                           entry.count, entry.in_use, entry.copy, lower, upper);
        saved += lower;
        saved_up += upper;
    }

    out << fmt::format("Size {}: {} < Saved < {} bytes (x{:6.2f} - {:6.2f})\n", size, saved,
                       saved_up, float(size + saved) / float(size),
                       float(size + saved_up) / float(size));
    out << fmt::format("Spent {} ms waiting on lock\n", wait_time);
    return out;
}

StringDatabase &StringDatabase::instance() {
    static StringDatabase db;
    return db;
}

StringView StringDatabase::operator[](std::size_t i) const {
    // we need to lock here in case the array gets reallocated
    // during a parallel insert
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();

    assert(i < strings.size(), "array out of bound");

    if (i < strings.size()) {
        [[likely]] return strings[i].data;
    }

    [[unlikely]] return StringView();
}

std::size_t StringDatabase::dec(std::size_t n) {
    if (n == 0) {
        return n;
    }

    // we need to lock here in case the array gets reallocated during a parallel insert
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();

    strings[n].in_use -= 1;

    // Easiest way to keep track of every StringRef in the code
    strings[n].copy += 1;
    return n;
}

std::size_t StringDatabase::inc(std::size_t i) {
    if (i == 0) {
        return i;
    }

    if (i >= strings.size()) {
        debug("Critical error {} < {}", i, strings.size());
        return 0;
    }

    // we need to lock here in case the array gets reallocated during a parallel insert
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();

    strings[i].in_use += 1;
    return i;
};

StringRef StringDatabase::string(String const &name) {
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();

    auto val = defined.find(name);

    if (val == defined.end()) {
        std::size_t n = strings.size();

        strings.push_back({name, 1, 0, 1});
        StringView str = strings[n].data;

        defined[str] = n;
        return StringRef(n);
    }

    auto ref = val->second;
    strings[ref].count += 1;
    strings[ref].in_use += 1;
    assert(ref < strings.size(), "StringRef should be valid");
    return StringRef(ref);
}

StringDatabase::StringDatabase() {
    strings.reserve(128);
    strings.push_back({"", 0, 0});
    StringView str = strings[0].data;
    defined[str]   = 0;
}

} // namespace lython
