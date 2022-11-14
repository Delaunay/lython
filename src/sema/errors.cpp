#include "sema/errors.h"
#include "ast/magic.h"
#include "ast/ops.h"
#include "utilities/names.h"
#include "utilities/strings.h"

namespace lython {

std::string TypeError::message(String const& lhs_v,
                               String const& lhs_t,
                               String const& rhs_v,
                               String const& rhs_t) {
    Array<String> msg = {"TypeError: "};
    if (lhs_v.size() > 0) {
        msg.push_back("expression `");
        msg.push_back((lhs_v));
        msg.push_back("` ");
    }
    if (lhs_v.size() > 0 && lhs_t.size() > 0) {
        msg.push_back("of ");
    }
    if (lhs_t.size() > 0) {
        msg.push_back("type `");
        msg.push_back((lhs_t));
        msg.push_back("` ");
    }

    msg.push_back("is not compatible with ");
    if (rhs_v.size() > 0) {
        msg.push_back("expression `");
        msg.push_back((rhs_v));
        msg.push_back("` ");
    }
    if (rhs_v.size() > 0 && rhs_t.size() > 0) {
        msg.push_back("of ");
    }
    if (rhs_t.size() > 0) {
        msg.push_back("type `");
        msg.push_back((rhs_t));
        msg.push_back("`");
    }
    return std::string(join("", msg));
}
std::string TypeError::message() const {
    auto _str = [](ExprNode* r) {
        if (r == nullptr)
            return String();
        return str(r);
    };
    return message(_str(lhs_v), _str(lhs_t), _str(rhs_v), _str(rhs_t));
}

std::string NameError::message() const {
    return fmt::format("NameError: name '{}' is not defined", str(name));
}

std::string AttributeError::message() const { return message(str(obj->name), str(attr)); }

std::string AttributeError::message(String const& name, String const& attr) {
    return fmt::format("AttributeError: '{}' has no attribute '{}'", name, attr);
}

std::string UnsupportedOperand::message() const { return message(operand, str(lhs_t), str(rhs_t)); }

std::string
UnsupportedOperand::message(String const& op, String const& lhs_t, String const& rhs_t) {
    return fmt::format(
        "TypeError: unsupported operand type(s) for {}: '{}' and '{}'", op, lhs_t, rhs_t);
}

std::string ModuleNotFoundError::message() const { return message(str(module)); }

std::string ModuleNotFoundError::message(String const& module) {
    return fmt::format("ModuleNotFoundError: No module named '{}'", module);
}

std::string ImportError::message() const { return message(str(module), str(name)); }

std::string ImportError::message(String const& module, String const& name) {
    return fmt::format("ImportError: cannot import name {} from '{}'", name, module);
}

std::string RecursiveDefinition::message() const { return message(fun, cls); }

std::string RecursiveDefinition::message(ExprNode const* fun, ClassDef const* cls) {
    return "RecursiveDefinition: ";
}

}  // namespace lython