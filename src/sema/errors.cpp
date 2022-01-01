#include "ast/magic.h"
#include "ast/ops.h"
#include "sema/errors.h"
#include "utilities/strings.h"

namespace lython {

std::string const &TypeError::message() const {
    if (cached_message != "") {
        return cached_message;
    }

    Array<String> msg = {};
    if (lhs_v) {
        msg.push_back("expression `");
        msg.push_back(str(lhs_v));
        msg.push_back("`");
        msg.push_back(" of ");
    }
    if (lhs_t) {
        msg.push_back("type `");
        msg.push_back(str(lhs_t));
        msg.push_back("`");
    } else {
        msg.push_back("type None");
    }

    msg.push_back(" is not compatible with ");
    if (rhs_v) {
        msg.push_back("expression `");
        msg.push_back(str(rhs_v));
        msg.push_back("`");
        msg.push_back(" of ");
    }
    if (rhs_t) {
        msg.push_back("type `");
        msg.push_back(str(rhs_t));
        msg.push_back("`");
    } else {
        msg.push_back("type None");
    }

    cached_message = std::string(join("", msg));
    return cached_message;
}

} // namespace lython