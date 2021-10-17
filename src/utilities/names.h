#ifndef LYTHON_SRC_AST_HEADER
#define LYTHON_SRC_AST_HEADER

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dtypes.h"
#include "logging/logging.h"
#include "utilities/stopwatch.h"

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

// Should be careful to only use this for name-like strings
// Since we keep the strings forever
// At the moment this is global but we should maybe tie this to a Module
// so the strings can expire
class StringDatabase {
    public:
    static StringDatabase &instance();

    StringView operator[](std::size_t i) const;

    StringRef string(String const &name);

    StringDatabase();

    std::ostream &report(std::ostream &out) const;

    private:
    std::size_t inc(std::size_t i);

    std::size_t dec(std::size_t n);

    struct StringEntry {
        String data;
        int    count  = 1;
        int    copy   = 0;
        int    in_use = 0;
    };

    Dict<StringView, std::size_t> defined; // Used to check if the string is already stored
    Array<StringEntry>            strings; // String storage
    mutable std::recursive_mutex  mu;
    mutable double                wait_time;

    friend class StringRef;
    friend bool _metadata_init_names();
};

// Very Cheap string reference
class StringRef {
    public:
    StringRef(std::size_t r = 0): ref(StringDatabase::instance().inc(r)) {
        assert(ref < StringDatabase::instance().strings.size(), "StringRef is valid");
    }

    StringRef(String const &name): ref(StringDatabase::instance().string(name).ref) {
        assert(ref < StringDatabase::instance().strings.size(), "StringRef is valid");
    }

    StringRef(StringRef const &name): ref(StringDatabase::instance().inc(name.ref)) {
        // assert(ref < StringDatabase::instance().strings.size(), "StringRef is valid");
    }

    StringRef(StringRef const &&name): ref(StringDatabase::instance().inc(name.ref)) {
        assert(ref < StringDatabase::instance().strings.size(), "StringRef is valid");
    }

    bool operator==(StringRef const &b) const { return ref == b.ref; }

    bool operator!=(StringRef const &b) const { return ref != b.ref; }

    StringRef &operator=(String const &name) {
        StringDatabase::instance().dec(ref);
        ref = StringDatabase::instance().string(name).ref;
        return *this;
    }

    StringRef operator=(StringRef const &name) {
        StringDatabase::instance().dec(ref);
        ref = StringDatabase::instance().inc(name.ref);
        return *this;
    }

    ~StringRef();

    String __str__() const;

    operator StringView() const;

    operator bool() const { return ref != 0; }

    std::size_t __id__() const { return ref; }

    private:
    std::size_t ref = 0;
};

std::ostream &operator<<(std::ostream &out, StringRef ref);

String join(String const &sep, Array<StringRef> const &strs);

// hash the reference instead of the string itself
// This could cause issues if we have multiple string databases
struct string_ref_hash {
    std::size_t            operator()(StringRef const &v) const noexcept { return _h(v.__id__()); }
    std::hash<std::size_t> _h;
};

} // namespace lython

#endif
