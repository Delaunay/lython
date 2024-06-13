
#include "ast/values/value.h"
#include "nodes.h"
#include "utilities/printing.h"

// #include "ast/meta.generated.h"
namespace lython {

ExprNode* None();
ExprNode* True();
ExprNode* False();

std::ostream& operator<<(std::ostream& out, UnaryOperator const& v) ;
std::ostream& operator<<(std::ostream& out, BinaryOperator const& v) ;
std::ostream& operator<<(std::ostream& out, BoolOperator const& v) ;
std::ostream& operator<<(std::ostream& out, CmpOperator const& v) ;

// --------------------------------------------------------------------
// to-string

String str(NodeKind k) {
#define CASEGEN(name)    \
    case NodeKind::name: \
        return #name;

#define X(name, _)     CASEGEN(name)
#define SECTION(name)  CASEGEN(name)
#define EXPR(name, _)  CASEGEN(name)
#define STMT(name, _)  CASEGEN(name)
#define MOD(name, _)   CASEGEN(name)
#define MATCH(name, _) CASEGEN(name)
#define VM(name, _) CASEGEN(name)

    switch (k) {
        NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH, VM)

    default: break;
    }

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM
    return "<invalid>";
}

std::ostream& operator<<(std::ostream& out, BoolOperator const& v) {
    switch (v) {
#define OP(name, kw, _)        \
    case BoolOperator::name: { \
        out << #kw;            \
        return out;                \
    }
        BOOL_OPERATORS(OP)

    default: break;
#undef OP
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, BinaryOperator const& v) {
    switch (v) {
#define OP(name, kw, _)          \
    case BinaryOperator::name: { \
        out << #name;            \
        return out;                  \
    }
        BINARY_OPERATORS(OP)

    default: break;
#undef OP
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, UnaryOperator const& v) {
    switch (v) {
#define OP(name, kw, _)         \
    case UnaryOperator::name: { \
        out << #name;           \
        return out;                 \
    }
        UNARY_OPERATORS(OP)

#undef OP
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, CmpOperator const& v) {
    switch (v) {
#define OP(name, kw, _)       \
    case CmpOperator::name: { \
        out << #name;         \
        return out;               \
    }
        COMP_OPERATORS(OP)

#undef OP
    }
    return out;
}

void ClassDef::Attr::dump(std::ostream& out) {
    out << name << ": " << str(type) << " = " << str(stmt);
}

StringRef operator_magic_name(BinaryOperator const& v, bool reverse) {

#define OP(name, kw, magic)                                \
    static StringRef m_##name  = String("__" #magic "__"); \
    static StringRef mr_##name = String("__r" #magic "__");

    BINARY_OPERATORS(OP)

#undef OP

    switch (v) {
#define OP(name, kw, _)                        \
    case BinaryOperator::name: {               \
        return reverse ? mr_##name : m_##name; \
    }
        BINARY_OPERATORS(OP)

#undef OP
    }

    return StringRef("");
}

StringRef operator_magic_name(BoolOperator const& v, bool reverse) {

#define OP(name, kw, magic)                                \
    static StringRef m_##name  = String("__" #magic "__"); \
    static StringRef mr_##name = String("__r" #magic "__");

    BOOL_OPERATORS(OP)

#undef OP

    switch (v) {
#define OP(name, kw, _)                        \
    case BoolOperator::name: {                 \
        return reverse ? mr_##name : m_##name; \
    }
        BOOL_OPERATORS(OP)

    default: break;
#undef OP
    }

    return StringRef("");
}

StringRef operator_magic_name(UnaryOperator const& v, bool reverse) {

#define OP(name, kw, magic)                                \
    static StringRef m_##name  = String("__" #magic "__"); \
    static StringRef mr_##name = String("__r" #magic "__");

    UNARY_OPERATORS(OP)

#undef OP

    switch (v) {
#define OP(name, kw, _)                        \
    case UnaryOperator::name: {                \
        return reverse ? mr_##name : m_##name; \
    }
        UNARY_OPERATORS(OP)

#undef OP
    }

    return StringRef("");
}

StringRef operator_magic_name(CmpOperator const& v, bool reverse) {

#define OP(name, kw, magic)                                \
    static StringRef m_##name  = String("__" #magic "__"); \
    static StringRef mr_##name = String("__r" #magic "__");

    COMP_OPERATORS(OP)

#undef OP

    switch (v) {
#define OP(name, kw, _)                        \
    case CmpOperator::name: {                  \
        return reverse ? mr_##name : m_##name; \
    }
        COMP_OPERATORS(OP)

#undef OP
    }

    return StringRef("");
}

// FIXME: this is a hack
// to avoid cicle we should just make a reference to the type that cycles
bool Arrow::add_arg_type(ExprNode* arg_type) {
    if (arg_type != this) {
        args.push_back(arg_type);

        if (has_circle(this)) {
            args.pop_back();
            return false;
        }

        return true;
    }
    kwwarn(outlog(), "trying to adding self to an arrow argument");
    return false;
}

bool Arrow::set_arg_type(int i, ExprNode* arg_type) {

    if (arg_type != this) {
        ExprNode* old = args[i];
        args[i]       = arg_type;

        if (has_circle(this)) {
            args[i] = old;
            return false;
        }

        return true;
    }

    kwwarn(outlog(), "trying to assing self to an arrow argument");
    return false;
}


template<typename T, typename Fun>
void static_visit(T* self, Fun fun) {
    using ArgumentIter_t = ArgumentIter<std::is_const_v<T>>;
    using Epxr_t = typename std::conditional<std::is_const_v<T>, ExprNode const*, ExprNode*>::type;
    using Arg_t = typename std::conditional<std::is_const_v<T>, Arg const&, Arg&>::type;


    int i = 0;
    int default_offset = int(self->defaults.size()) - (
        self->posonlyargs.size() + self->args.size());

    auto getdefault = [&]() -> int{
        return default_offset + i;
    };
    
    for(Arg_t posonly: self->posonlyargs) {
        Epxr_t value = nullptr;
        auto offset = getdefault();
        if (offset >= 0 && offset < self->defaults.size()) {
            value = self->defaults[offset];
        }
        fun(ArgumentIter_t{ArgumentKind::PosOnly, posonly, value});
        i += 1;
    }

    for(Arg_t arg: self->args) {
        Epxr_t value = nullptr;
        
        auto offset = getdefault();
        if (offset >= 0 && offset < self->defaults.size()) {
            value = self->defaults[offset];
        }
        fun(ArgumentIter_t{ArgumentKind::Regular, arg, value});
        i += 1;
    }

    if (self->vararg.has_value()) {
        fun(ArgumentIter_t{ArgumentKind::VarArg, self->vararg.value(), nullptr});
        i += 1;
    }

    int kw = 0;
    for(Arg_t kwonlyarg: self->kwonlyargs) {
        Epxr_t value = nullptr;

        auto offset = int(self->kw_defaults.size()) - int(self->kwonlyargs.size()) + kw;
        if (offset >= 0 && offset < self->kw_defaults.size()) {
            value = self->kw_defaults[offset];
        }

        fun(ArgumentIter_t{ArgumentKind::KwOnly, kwonlyarg, value});
        i += 1;
        kw += 1;
    }

        if (self->kwarg.has_value()) {
        fun(ArgumentIter_t{ArgumentKind::KwArg, self->kwarg.value(), nullptr});
        i += 1;
    }
}


    void Arguments::visit(std::function<void(ArgumentIter<false> const&)> fun) {
        static_visit(this, fun);
    }
    void Arguments::visit(std::function<void(ArgumentIter<true> const&)> fun) const {
        static_visit(this, fun);
    }
// ------------------------------------------
}  // namespace lython
