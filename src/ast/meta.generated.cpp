#include "dtypes.h"
#include "ast/nodes.h"
#include "utilities/names.h"

namespace lython {
template <>
struct meta::ReflectionTrait<MyStruct1> {
    static int register_members() {
        meta::register_property<&MyStruct1::b>("b");
        return 1;
    }
};
}
