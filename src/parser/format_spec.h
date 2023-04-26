#pragma once

#include "dtypes.h"

namespace lython {

struct FormatSpecifier {
    char fill          = '\0';
    char align         = '\0';  // < > = ^
    char sign          = '\0';  // + - ' '
    char alternate     = '\0';  // # 
    char pad           = '\0';  // 0
    uint64_t width     = 0;
    uint64_t precision = 0;
    char type          = '\0';  // b, c, d, o, x, X, n, e, E, f, F, g, G, n, %
    
    bool valid = false;
    String unparsed;

    String __str__() const;
    String __repr__() const ;

    static FormatSpecifier parse(String const& buffer);

    bool is_float() const;
    bool is_integer() const;
    bool is_undefined() const;
};
} 