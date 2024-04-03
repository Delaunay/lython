
#include "native.h"
#include "ast/constant.h"

namespace lython {

struct ConstantValue NativeObject::cmember(int member_id) {
        if (!is_valid()) {
            lyassert(0, "Underlying object is null");
            return ConstantValue();
        }
        meta::Member const& member = _get_member(member_id);

        ConstantValue value;
        // we need to keep the size of the member as well
        int8* address = (_memory() + member.offset);
        int byte_count = member.size;

        memcpy((void*)&value.value, address, byte_count);

        #define POD(a, native, c)                               \
        if (member.type == meta::type_id<native>()) {           \
            value.kind = ConstantValue::T##a;                   \
            return value;                                       \
        }
        
        #define CPX(a, b, c)

        ConstantType(POD, CPX)

        return value;
    }
}