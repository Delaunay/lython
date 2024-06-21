#ifndef LYTHON_SRC_AST_HEADER
#define LYTHON_SRC_AST_HEADER

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/core.h>

#include "dtypes.h"
#include "logging/exceptions.h"
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
    bool print_stats = false;

    static StringDatabase& instance();

    StringView operator[](std::size_t i) const;

    StringRef string(String const& name);

    StringDatabase();

    ~StringDatabase() {
        if (print_stats) {
            report(std::cout);
        }
    }

    std::ostream& report(std::ostream& out) const;

    struct StringEntry {
        String data;
        int    count  = 1;
        int    copy   = 0;
        int    in_use = 0;
    };

    private:
    StringRef lookup_or_insert_string(String const& name);
    StringRef insert_string(String const& name);

    std::size_t inc(std::size_t i);

    std::size_t dec(std::size_t n);

    std::size_t count() const { return size; }

    Array<StringEntry>& newblock();
    Array<StringEntry>& current_block();

    StringEntry& get(std::size_t i) {
        size_t block = i / block_size;
        size_t entry = i % block_size;

        if (block < reverse.size()){
            return (*reverse[block])[entry];
        }
        return (*reverse[0])[0];
    }

    StringEntry const& get(std::size_t i) const {
        size_t block = i / block_size;
        size_t entry = i % block_size;
        return (*reverse[block])[entry];
    }

// Array<StringEntry>            strings; // String storage
#if !BUILD_WEBASSEMBLY
    mutable std::recursive_mutex mu;
#endif
    mutable double wait_time;

    friend class StringRef;
    friend bool _metadata_init_names();

    // Used to check if the string is already stored
    Dict<StringView, std::size_t> defined;

    // Allocates strings in block to avoid reallocation
    int         block_size = 1024;
    std::size_t size       = 0;

    List<Array<StringEntry>>   memory_blocks;
    Array<Array<StringEntry>*> reverse;

    friend bool _metadata_init_names();
};

#define STRING_VIEW(X) X

// Very Cheap string reference
class StringRef {
    public:
    StringRef():
        StringRef(std::size_t(0))
    {}

    explicit StringRef(std::size_t r): ref(StringDatabase::instance().inc(r)) {
        lyassert(ref < StringDatabase::instance().count(), "StringRef is valid");
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
    }

    explicit StringRef(const char* str): StringRef(String(str))
    {
    }

    StringRef(String const& name): ref(StringDatabase::instance().string(name).ref) {
        lyassert(ref < StringDatabase::instance().count(), "StringRef is valid");
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
    }

    StringRef(StringRef const& name): ref(StringDatabase::instance().inc(name.ref)) {
        lyassert(ref < StringDatabase::instance().count(), "StringRef is valid");
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
    }

    StringRef(StringRef const&& name): ref(StringDatabase::instance().inc(name.ref)) {
        lyassert(ref < StringDatabase::instance().count(), "StringRef is valid");
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
    }

    bool operator==(StringRef const& b) const { return ref == b.ref; }

    bool operator!=(StringRef const& b) const { return ref != b.ref; }

    StringRef& operator=(String const& name) {
        StringDatabase::instance().dec(ref);
        ref = StringDatabase::instance().string(name).ref;
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
        return *this;
    }

    StringRef operator=(StringRef const& name) {
        StringDatabase::instance().dec(ref);
        ref = StringDatabase::instance().inc(name.ref);
        STRING_VIEW(debug_view = StringDatabase::instance()[ref]);
        return *this;
    }

    ~StringRef();

    void print(std::ostream& out) const;

    operator StringView() const;

    operator bool() const { return ref != 0; }

    std::size_t __id__() const { return ref; }

    private:
    std::size_t ref = 0;
    StringView  debug_view;
};

std::ostream& operator<<(std::ostream& out, StringRef ref);

String join(String const& sep, Array<StringRef> const& strs);

// hash the reference instead of the string itself
// This could cause issues if we have multiple string databases
struct string_ref_hash {
    std::size_t            operator()(StringRef const& v) const noexcept { return _h(v.__id__()); }
    std::hash<std::size_t> _h;
};

inline void show_string_stats_on_destroy(bool enabled) {
    StringDatabase::instance().print_stats = enabled;
}


struct Field {
    Field(StringRef name, std::size_t o, std::size_t s, StringRef type_name):
        name(name), offset(o), size(s), type_name(type_name)
    {}

    StringRef name;
    std::size_t offset;
    std::size_t size;
    StringRef type_name;
};

}  // namespace lython


template <>
struct std::hash<lython::StringRef> {
    std::size_t operator()(lython::StringRef const& s) const noexcept {
        return std::hash<std::size_t>{}(s.__id__());
    }
};


#endif
