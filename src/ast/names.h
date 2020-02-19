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

#define ASSERT(pred, msg)                                                      \
    {                                                                          \
        if (!(pred)) {                                                         \
            throw NameTaken(msg);                                              \
        }                                                                      \
    }


class StringRef;

StringRef get_string(String const& str);

std::ostream& operator<< (std::ostream& out, StringRef ref);

// Very Cheap string reference
class StringRef{
public:
    StringRef(std::size_t ref = 0):
        ref(ref)
    {}

    StringRef(String const& name):
        ref(get_string(name).ref)
    {}

    bool operator== (StringRef const& b) const {
        return ref == b.ref;
    }

    String str() const;

    operator StringView() const;

    std::size_t ref = 0;
};

// hash the reference instead of the string itself
// This could cause issues if we have multiple string databases
struct string_ref_hash {
    std::size_t operator()(StringRef const &v) const noexcept{
        return _h(v.ref);
    }
    std::hash<std::size_t> _h;
};


// Should be careful to only use this for name like strings
// Since we keep the strings forever
// At the moment this is global but we should maybe tie this to a Module
// so the strings can expire
class StringDatabase{
public:
    static StringDatabase& instance(){
        static StringDatabase db;
        return db;
    }

    StringView operator[] (std::size_t i){
        return strings[i];
    }

    StringRef string(String const& name){
        std::size_t val = defined[name];
        if (val == 0){
            std::size_t n = strings.size();
            strings.push_back(name);
            count.push_back(1);

            StringView str = strings[n];
            defined[str] = n;
            return StringRef(n);
        }

        count[val] += 1;
        return StringRef(val);
    }

    StringDatabase(){
        strings.reserve(128);
        strings.push_back("");
    }

    std::ostream& report(std::ostream& out){
        std::size_t saved = 0;
        for(auto i = 0u; i < count.size(); ++i){
            auto s = strings[i].length() * count[i];
            out << fmt::format("| {:30} | {:4} | {:4} |\n", strings[i], count[i], s);
            saved += s;
        }
        out << fmt::format("Saved: {} bytes\n", saved);
        return out;
    }

private:
    Dict<StringView, std::size_t> defined;
    Array<String>                 strings;
    Array<int>                    count;
};

}

#endif
