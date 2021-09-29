#include "names.h"

namespace lython {
StringRef get_string(String const &str) { return StringDatabase::instance().string(str); }

std::ostream &operator<<(std::ostream &out, StringRef ref) {
    return out << StringDatabase::instance()[ref.ref];
}

String StringRef::__str__() const {
    auto view = StringDatabase::instance()[ref];
    return String(view);
}

StringRef::operator StringView() const { return StringDatabase::instance()[ref]; }

StringRef::~StringRef() { StringDatabase::instance().strings[ref].in_use += 1; }

std::ostream &StringDatabase::report(std::ostream &out) const {
    std::size_t saved    = 0;
    std::size_t saved_up = 0;
    std::size_t size     = 9;

    out << fmt::format("| {:30} | {:4} | {:4} | {:4} | {:4} |\n", "str", "#", "use", "low", "upp");

    for (auto &entry: strings) {
        size += entry.data.size();
        auto lower = entry.data.size() * (entry.count - 1);
        auto upper = entry.data.size() * (entry.in_use - 1);

        out << fmt::format("| {:30} | {:4} | {:4} | {:4} | {:4} |\n", entry.data, entry.count,
                           entry.in_use, lower, upper);
        saved += lower;
        saved_up += upper;
    }

    out << fmt::format("Size {}: {} < Saved < {} bytes (x{:6.2f} - {:6.2f})\n", size, saved,
                       saved_up, float(size + saved) / float(size),
                       float(size + saved_up) / float(size));
    return out;
}
} // namespace lython
