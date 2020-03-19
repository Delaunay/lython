#ifndef LYTHON_UTILITIES_STRINGS_HEADER
#define LYTHON_UTILITIES_STRINGS_HEADER

#include "dtypes.h"

namespace lython {

String join(String const& sep, Array<String> const& strs);

String strip(String const& v);
}

#endif
