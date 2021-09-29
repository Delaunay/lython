#ifndef LYTHON_SRC_AST_HEADER
#define LYTHON_SRC_AST_HEADER

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dtypes.h"
#include "logging/logging.h"

namespace lython {
// /!\ string a allocated twice

NEW_EXCEPTION(NameTaken);

#define ASSERT(pred, msg)         \
    {                             \
        if (!(pred)) {            \
            throw NameTaken(msg); \
        }                         \
    }

class StringRef;

StringRef get_string(String const &str);

std::ostream &operator<<(std::ostream &out, StringRef ref);

// Very Cheap string reference
class StringRef {
    public:
    StringRef(std::size_t ref = 0): ref(ref) {}

    StringRef(String const &name): ref(get_string(name).ref) {}

    bool operator==(StringRef const &b) const { return ref == b.ref; }

    bool operator!=(StringRef const &b) const { return ref != b.ref; }

    ~StringRef();

    String __str__() const;

    operator StringView() const;

    std::size_t ref = 0;

    operator bool() const { return ref != 0; }
};

String join(String const &sep, Array<StringRef> const &strs);

// hash the reference instead of the string itself
// This could cause issues if we have multiple string databases
struct string_ref_hash {
    std::size_t            operator()(StringRef const &v) const noexcept { return _h(v.ref); }
    std::hash<std::size_t> _h;
};

// Should be careful to only use this for name-like strings
// Since we keep the strings forever
// Currently this is only used when setting strings inside our AST
class StringDatabase {
    public:
    static StringDatabase &instance() {
        static StringDatabase db;
        return db;
    }

    StringView operator[](std::size_t i) { return strings[i].data; }

    StringRef string(String const &name) {
        auto val = defined.find(name);

        if (val == defined.end()) {
            std::size_t n = strings.size();

            strings.push_back({name, 1, 0});
            StringView str = strings[n].data;

            defined[str] = n;
            return StringRef(n);
        }

        auto ref = val->second;
        strings[ref].count += 1;
        return StringRef(ref);
    }

    StringDatabase() {
        strings.reserve(128);

        strings.push_back({"", 0, 0});
        StringView str = strings[0].data;
        defined[str]   = 0;
    }

    std::ostream &report(std::ostream &out) const;

    public:
    struct StringEntry {
        String data;
        int    count  = 1;
        int    in_use = 0;
    };

    private:
    Dict<StringView, std::size_t> defined; // Used to check if the string is already stored
    Array<StringEntry>            strings; // String storage

    friend class StringRef;
};

} // namespace lython

#endif
