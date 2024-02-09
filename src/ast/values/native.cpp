
#include "native.h"
#include "ast/constant.h"

namespace lython {

struct ConstantValue NativeObject::cmember(int member_id) {
        if (!is_valid()) {
            assert(0, "Underlying object is null");
            return ConstantValue();
        }
        meta::Member const& member = _get_member(member_id);

        ConstantValue value;
        // we need to keep the size of the member as well
        int8* address = (_memory() + member.offset);


        // value.kind = member.type;
        // (value.value.u64) = ;
        return value;
    }
}