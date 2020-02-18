#ifndef LYTHON_METADATA_H
#define LYTHON_METADATA_H

#include "dtypes.h"
#include "ast/nodes.h"

namespace lython {

#define TYPES_METADATA(X)\
    X(String, String)\
    X(char, Char)\
    X(AST::MathNode, MathNode)


//X(int8, Int8)\
//X(int16, Int16)\
//X(int32, Int32)\
//X(int64, Int64)\
//X(uint8, UInt8)\
//X(uint16, UInt16)\
//X(uint32, UInt32)\
//X(uint64, UInt64)\
//X(float32, Float32)\
//X(float64, Float64)\
//

#define DEFINE_METADATA(type, tname)\
    template <>\
    inline const char* type_name<type>() {\
        static const char* name = _insert_typename<type>(#tname);\
        return name;\
    }

TYPES_METADATA(DEFINE_METADATA)

void metadata_init_names();

} // namespace lython

#endif
