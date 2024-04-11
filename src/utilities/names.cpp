#include "names.h"
#include "dependencies/coz_wrap.h"
#include "logging/logging.h"

namespace lython {

std::ostream& operator<<(std::ostream& out, StringRef ref) {
    return out << StringDatabase::instance()[ref.__id__()];
}

void StringRef::print(std::ostream& out) const {
    auto view = StringDatabase::instance()[ref];
    out << String(view);
}

StringRef::operator StringView() const { return StringDatabase::instance()[ref]; }

StringRef::~StringRef() { StringDatabase::instance().dec(ref); }

std::ostream& StringDatabase::report(std::ostream& out) const {
    std::size_t saved    = 0;
    std::size_t saved_up = 0;
    std::size_t size     = 9;

    out << fmt::format(
        "| {:30} | {:4} | {:4} | {:4} | {:4} | {:4} |\n", "str", "#", "use", "cpy", "low", "upp");

    for (auto& strings: memory_blocks) {
        for (auto& entry: strings) {
            size += entry.data.size();
            auto lower = entry.data.size() * (entry.count - 1);
            auto upper = entry.data.size() * (entry.copy - 1);
            saved += lower;
            saved_up += upper;

            if (entry.count == 1 && entry.in_use == 0) {
                continue;
            }

            out << fmt::format("| {:30} | {:4} | {:4} | {:4} | {:4} | {:4} |\n",
                               entry.data,
                               entry.count,
                               entry.in_use,
                               entry.copy,
                               lower,
                               upper);
        }
    }

    out << fmt::format("Size {}: {} < Saved < {} bytes (x{:6.2f} - {:6.2f})\n",
                       size,
                       saved,
                       saved_up,
                       float(size + saved) / float(size),
                       float(size + saved_up) / float(size));
    out << fmt::format("Spent {} ms waiting on lock\n", wait_time);

    return out;
}

StringDatabase& StringDatabase::instance() {
    static StringDatabase db;
    return db;
}

StringView StringDatabase::operator[](std::size_t i) const {
// we need to lock here in case the array gets reallocated
// during a parallel insert
#if !BUILD_WEBASSEMBLY
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();
#endif

    lyassert(i < size, "array out of bound");

    if (i < size) {
        [[likely]] return get(i).data;
    }

    [[unlikely]] return StringView();
}

std::size_t StringDatabase::dec(std::size_t n) {
    if (n == 0) {
        return n;
    }

#if !BUILD_WEBASSEMBLY
    // we need to lock here in case the array gets reallocated during a parallel insert
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();
#endif

    // Easiest way to keep track of every StringRef in the code
    auto& entry = get(n);
    entry.in_use -= 1;
    entry.copy += 1;

    return n;
}

std::size_t StringDatabase::inc(std::size_t i) {
    if (i == 0) {
        return i;
    }

    if (i >= size) {
        kwdebug(outlog(), "Critical error {} < {}", i, size);
        return 0;
    }

#if !BUILD_WEBASSEMBLY
    // we need to lock here in case the array gets reallocated during a parallel insert
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();
#endif

    get(i).in_use += 1;
    return i;
};

Array<StringDatabase::StringEntry>& StringDatabase::current_block() {
    Array<StringEntry>& last = *memory_blocks.rbegin();

    if (last.capacity() != last.size())
        return last;

    return newblock();
}

StringRef StringDatabase::string(String const& name) {
    COZ_BEGIN("T::StringDatabase::string");
    auto str = lookup_or_insert_string(name);

    COZ_PROGRESS_NAMED("StringDatabase::string");
    COZ_END("T::StringDatabase::string");
    return str;
}

StringRef StringDatabase::insert_string(String const& name) {
    COZ_BEGIN("T::StringDatabase::insert");
    std::size_t id      = size;
    auto&       strings = current_block();
    std::size_t n       = strings.size();

    strings.push_back({name, 1, 0, 1});
    StringView str = strings[n].data;

    defined[str] = {id};
    size += 1;

    COZ_PROGRESS_NAMED("StringDatabase::insert");
    COZ_END("T::StringDatabase::insert");
    return StringRef(id);
}

StringRef StringDatabase::lookup_or_insert_string(String const& name) {

#if !BUILD_WEBASSEMBLY
    StopWatch<>                           timer;
    std::lock_guard<std::recursive_mutex> guard(mu);
    wait_time += timer.stop();
#endif

    auto val = defined.find(name);

    if (val == defined.end()) {
        return insert_string(name);
    }

    auto ref   = val->second;
    auto entry = get(ref);
    entry.count += 1;
    entry.in_use += 1;

    return StringRef(ref);
}

Array<StringDatabase::StringEntry>& StringDatabase::newblock() {
    memory_blocks.push_back(Array<StringEntry>());
    Array<StringEntry>& last = *memory_blocks.rbegin();
    last.reserve(block_size);
    reverse.push_back(&last);
    return last;
}

StringDatabase::StringDatabase() {

    auto& block = newblock();
    block.push_back({"", 0, 0});
    StringView str = block[0].data;

    defined[str] = {0};
    size         = 1;
}

}  // namespace lython
